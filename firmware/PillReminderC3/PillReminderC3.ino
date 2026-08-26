#include <Arduino.h>

// Pill Reminder v0.1.4 hardware bring-up build.
// Deliberately excludes Wi-Fi, clock, flash, HTTP, Serial, and PWM.

constexpr uint8_t LED_PINS[3][3] = {
  // Red, Green, Blue
  {0, 1, 3},
  {4, 5, 6},
  {7, 10, 20}
};
constexpr uint8_t BUTTON_PIN = 21;
constexpr uint32_t DEBOUNCE_MS = 35;

bool taken = false;
bool rawButtonDown = false;
bool stableButtonDown = false;
uint32_t rawButtonChangedMs = 0;

void setLed(uint8_t led, bool red, bool green, bool blue) {
  digitalWrite(LED_PINS[led][0], red ? HIGH : LOW);
  digitalWrite(LED_PINS[led][1], green ? HIGH : LOW);
  digitalWrite(LED_PINS[led][2], blue ? HIGH : LOW);
}

void allOff() {
  for (uint8_t led = 0; led < 3; ++led) setLed(led, false, false, false);
}

void allGreen() {
  for (uint8_t led = 0; led < 3; ++led) setLed(led, false, true, false);
}

void showOneChannelTest() {
  // Nine 1.2-second slots: 900 ms illuminated, 300 ms completely dark.
  const uint32_t now = millis();
  const uint8_t step = (now / 1200) % 9;
  const bool illuminated = (now % 1200) < 900;

  allOff();
  if (!illuminated) return;

  const uint8_t led = step / 3;
  const uint8_t color = step % 3;
  setLed(led, color == 0, color == 1, color == 2);
}

void serviceButton() {
  const uint32_t now = millis();
  const bool down = digitalRead(BUTTON_PIN) == LOW;

  if (down != rawButtonDown) {
    rawButtonDown = down;
    rawButtonChangedMs = now;
  }

  if (now - rawButtonChangedMs < DEBOUNCE_MS || down == stableButtonDown) return;

  stableButtonDown = down;
  if (!stableButtonDown) taken = !taken;  // Toggle on release for testing.
}

void setup() {
  for (uint8_t led = 0; led < 3; ++led) {
    for (uint8_t color = 0; color < 3; ++color) {
      pinMode(LED_PINS[led][color], OUTPUT);
      digitalWrite(LED_PINS[led][color], LOW);
    }
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  allOff();
}

void loop() {
  serviceButton();
  if (taken) allGreen();
  else showOneChannelTest();
  delay(1);
}
