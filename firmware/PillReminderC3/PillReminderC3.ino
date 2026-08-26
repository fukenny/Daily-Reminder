#include <Arduino.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

#include "PillReminderConfig.h"

// Pill Reminder v0.3.0 release candidate.
// Three common-cathode RGB LEDs and one normally-open button.

constexpr uint8_t LED_PINS[3][3] = {
  // Red, Green, Blue
  {0, 1, 3},
  {4, 5, 6},
  {7, 10, 20}
};
constexpr uint8_t BUTTON_PIN = 21;
constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t CLOCK_CHECK_MS = 1000;
constexpr uint32_t WIFI_RETRY_MS = 30000;
constexpr uint32_t TIME_RETRY_MS = 15000;

struct Color {
  bool red;
  bool green;
  bool blue;
};

constexpr Color OFF     = {false, false, false};
constexpr Color RED     = {true,  false, false};
constexpr Color GREEN   = {false, true,  false};
constexpr Color BLUE    = {false, false, true};
constexpr Color YELLOW  = {true,  true,  false};
constexpr Color CYAN    = {false, true,  true};
constexpr Color MAGENTA = {true,  false, true};
constexpr Color WHITE   = {true,  true,  true};

Preferences preferences;
bool taken = false;
bool clockReady = false;
bool pendingConfirmation = false;
bool rawButtonDown = false;
bool stableButtonDown = false;
uint32_t rawButtonChangedMs = 0;
uint32_t lastClockCheckMs = 0;
uint32_t lastWifiAttemptMs = 0;
uint32_t lastTimeAttemptMs = 0;
String activeCycle;

bool placeholder(const char* value, const char* marker) {
  return value == nullptr || value[0] == '\0' || strstr(value, marker) != nullptr;
}

bool wifiConfigured() {
  return !placeholder(WIFI_SSID, "YOUR_WIFI") &&
         !placeholder(WIFI_PASSWORD, "YOUR_WIFI");
}

bool loggingConfigured() {
  return !placeholder(WEBHOOK_URL, "PASTE_") &&
         !placeholder(DEVICE_TOKEN, "CHANGE_THIS");
}

void setLed(uint8_t led, Color color) {
  digitalWrite(LED_PINS[led][0], color.red ? HIGH : LOW);
  digitalWrite(LED_PINS[led][1], color.green ? HIGH : LOW);
  digitalWrite(LED_PINS[led][2], color.blue ? HIGH : LOW);
}

void setThree(Color first, Color second, Color third) {
  setLed(0, first);
  setLed(1, second);
  setLed(2, third);
}

void showAttentionPattern(uint32_t nowMs) {
  const uint32_t phase = nowMs % 8000;

  if (phase < 2400) {
    constexpr Color rainbow[] = {RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA};
    const uint8_t tick = phase / 120;
    setThree(rainbow[(tick + 0) % 6],
             rainbow[(tick + 2) % 6],
             rainbow[(tick + 4) % 6]);
  } else if (phase < 4000) {
    const bool flip = ((phase - 2400) / 100) % 2;
    setThree(flip ? RED : BLUE,
             flip ? BLUE : RED,
             ((phase - 2400) / 200) % 2 ? WHITE : OFF);
  } else if (phase < 5200) {
    const uint8_t tick = (phase - 4000) / 150;
    constexpr Color mixed[] = {YELLOW, CYAN, MAGENTA};
    setThree(mixed[(tick + 0) % 3],
             mixed[(tick + 1) % 3],
             mixed[(tick + 2) % 3]);
  } else if (phase < 6400) {
    const bool flash = ((phase - 5200) / 75) % 2 == 0;
    setThree(flash ? WHITE : OFF, flash ? WHITE : OFF, flash ? WHITE : OFF);
  } else {
    const uint8_t tick = (phase - 6400) / 130;
    const uint8_t active = tick % 3;
    const Color color = (tick / 3) % 3 == 0 ? RED
                      : (tick / 3) % 3 == 1 ? GREEN
                                            : BLUE;
    setThree(active == 0 ? color : OFF,
             active == 1 ? color : OFF,
             active == 2 ? color : OFF);
  }
}

void showConfirmedGreen() {
  // Human vision is nonlinear: 12% duty looks much dimmer than the former 50%.
  const uint16_t duty = constrain(CONFIRMED_GREEN_BRIGHTNESS, 1, 100);
  const bool greenOn = (micros() % 1000) < duty * 10;
  const Color dimGreen = {false, greenOn, false};
  setThree(dimGreen, dimGreen, dimGreen);
}

String jsonEscape(const String& input) {
  String output;
  output.reserve(input.length() + 8);
  for (size_t i = 0; i < input.length(); ++i) {
    const char c = input[i];
    if (c == '\\' || c == '"') output += '\\';
    if (c == '\n' || c == '\r') output += ' ';
    else output += c;
  }
  return output;
}

String cycleKey(time_t now) {
  struct tm localTime;
  localtime_r(&now, &localTime);
  if (localTime.tm_hour < RESET_HOUR) {
    now -= 24 * 60 * 60;
    localtime_r(&now, &localTime);
  }

  char key[11];
  strftime(key, sizeof(key), "%Y-%m-%d", &localTime);
  return String(key);
}

bool readClock(time_t& now) {
  time(&now);
  return now > 1700000000;  // Reject the ESP32's unsynchronized epoch.
}

void saveState() {
  preferences.putString("cycle", activeCycle);
  preferences.putBool("taken", taken);
}

void applyCurrentCycle(time_t now) {
  const String current = cycleKey(now);
  if (current == activeCycle) return;

  const String savedCycle = preferences.getString("cycle", "");
  activeCycle = current;
  if (pendingConfirmation) {
    taken = true;
    pendingConfirmation = false;
  } else {
    taken = savedCycle == current && preferences.getBool("taken", false);
  }
  saveState();
}

void postTakenEvent(time_t now) {
  if (!loggingConfigured() || WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(5000);
  if (!http.begin(client, WEBHOOK_URL)) return;

  http.addHeader("Content-Type", "application/json");
  String body = "{\"token\":\"" + jsonEscape(DEVICE_TOKEN) +
                "\",\"epoch\":" + String(static_cast<unsigned long>(now)) +
                ",\"cycle\":\"" + jsonEscape(activeCycle) +
                "\",\"event\":\"TAKEN\",\"device\":\"" +
                jsonEscape(DEVICE_NAME) + "\",\"firmware\":\"" +
                jsonEscape(FIRMWARE_VERSION) + "\"}";
  http.POST(body);
  http.end();
}

void confirmTaken() {
  if (taken) return;

  time_t now = 0;
  if (readClock(now)) {
    clockReady = true;
    applyCurrentCycle(now);
  }

  taken = true;
  if (activeCycle.length() > 0) saveState();
  else pendingConfirmation = true;
  postTakenEvent(now);
}

void serviceButton() {
  const uint32_t nowMs = millis();
  const bool down = digitalRead(BUTTON_PIN) == LOW;

  if (down != rawButtonDown) {
    rawButtonDown = down;
    rawButtonChangedMs = nowMs;
  }

  if (nowMs - rawButtonChangedMs < DEBOUNCE_MS || down == stableButtonDown) return;
  stableButtonDown = down;
  if (!stableButtonDown) confirmTaken();
}

void startTimeSync() {
  configTzTime(TIME_ZONE, NTP_SERVER_1, NTP_SERVER_2);
  lastTimeAttemptMs = millis();
}

void serviceNetworkAndClock() {
  const uint32_t nowMs = millis();

  if (wifiConfigured() && WiFi.status() != WL_CONNECTED &&
      nowMs - lastWifiAttemptMs >= WIFI_RETRY_MS) {
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWifiAttemptMs = nowMs;
  }

  if (WiFi.status() == WL_CONNECTED && !clockReady &&
      nowMs - lastTimeAttemptMs >= TIME_RETRY_MS) {
    startTimeSync();
  }

  if (nowMs - lastClockCheckMs < CLOCK_CHECK_MS) return;
  lastClockCheckMs = nowMs;

  time_t now = 0;
  if (!readClock(now)) return;
  clockReady = true;
  applyCurrentCycle(now);
}

void setup() {
  for (uint8_t led = 0; led < 3; ++led) {
    for (uint8_t color = 0; color < 3; ++color) {
      pinMode(LED_PINS[led][color], OUTPUT);
      digitalWrite(LED_PINS[led][color], LOW);
    }
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  preferences.begin("pill-reminder", false);
  WiFi.mode(WIFI_STA);
  if (wifiConfigured()) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWifiAttemptMs = millis();
    startTimeSync();
  }
}

void loop() {
  serviceButton();
  serviceNetworkAndClock();
  if (taken) showConfirmedGreen();
  else showAttentionPattern(millis());
  yield();
}
