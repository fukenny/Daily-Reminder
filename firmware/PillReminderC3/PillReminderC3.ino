#include <Arduino.h>

// Pill Reminder v0.2.0 light-show build.
// This checkpoint tests the attention animation and confirmation button only.

constexpr uint8_t LED_PINS[3][3] = {
  // Red, Green, Blue
  {0, 1, 3},
  {4, 5, 6},
  {7, 10, 20}
};
constexpr uint8_t BUTTON_PIN = 21;
constexpr uint32_t DEBOUNCE_MS = 35;

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

bool taken = false;
bool rawButtonDown = false;
bool stableButtonDown = false;
uint32_t rawButtonChangedMs = 0;

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
    // Fast six-color chase.
    constexpr Color rainbow[] = {RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA};
    const uint8_t tick = phase / 120;
    setThree(
      rainbow[(tick + 0) % 6],
      rainbow[(tick + 2) % 6],
      rainbow[(tick + 4) % 6]
    );
  } else if (phase < 4000) {
    // Alternating red/blue emergency lights with a white accent.
    const bool flip = ((phase - 2400) / 100) % 2;
    setThree(
      flip ? RED : BLUE,
      flip ? BLUE : RED,
      ((phase - 2400) / 200) % 2 ? WHITE : OFF
    );
  } else if (phase < 5200) {
    // Rotating mixed-color blocks.
    const uint8_t tick = (phase - 4000) / 150;
    constexpr Color mixed[] = {YELLOW, CYAN, MAGENTA};
    setThree(
      mixed[(tick + 0) % 3],
      mixed[(tick + 1) % 3],
      mixed[(tick + 2) % 3]
    );
  } else if (phase < 6400) {
    // Sharp white strobe.
    const bool flash = ((phase - 5200) / 75) % 2 == 0;
    setThree(flash ? WHITE : OFF, flash ? WHITE : OFF, flash ? WHITE : OFF);
  } else {
    // Comet chase with color changes and dark space.
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
  // Approximately 50% brightness at 1 kHz. Only green channels participate.
  const bool greenOn = (micros() % 1000) < 500;
  const Color dimGreen = {false, greenOn, false};
  setThree(dimGreen, dimGreen, dimGreen);
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
  if (!stableButtonDown) taken = true;
}

void setup() {
  for (uint8_t led = 0; led < 3; ++led) {
    for (uint8_t color = 0; color < 3; ++color) {
      pinMode(LED_PINS[led][color], OUTPUT);
      digitalWrite(LED_PINS[led][color], LOW);
    }
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  serviceButton();
  if (taken) showConfirmedGreen();
  else showAttentionPattern(millis());
  yield();
}
