#include <Arduino.h>

// Direct-output diagnostic for the Pill Reminder ESP32-C3 wiring.
// This sketch intentionally contains no Wi-Fi, PWM, animation, or button code.

struct Channel {
  uint8_t gpio;
  const char *label;
};

constexpr Channel CHANNELS[] = {
  {0,  "LED 1 RED"},
  {1,  "LED 1 GREEN"},
  {3,  "LED 1 BLUE"},
  {4,  "LED 2 RED"},
  {5,  "LED 2 GREEN"},
  {6,  "LED 2 BLUE"},
  {7,  "LED 3 RED"},
  {10, "LED 3 GREEN"},
  {20, "LED 3 BLUE"},
};

constexpr uint8_t CHANNEL_COUNT = sizeof(CHANNELS) / sizeof(CHANNELS[0]);

void allOff() {
  for (uint8_t i = 0; i < CHANNEL_COUNT; ++i) {
    digitalWrite(CHANNELS[i].gpio, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  for (uint8_t i = 0; i < CHANNEL_COUNT; ++i) {
    pinMode(CHANNELS[i].gpio, OUTPUT);
  }
  allOff();

  Serial.println("RGB hardware test starting");
  Serial.println("Exactly one color in exactly one LED should light per step.");
}

void loop() {
  for (uint8_t i = 0; i < CHANNEL_COUNT; ++i) {
    allOff();
    Serial.printf("GPIO %u -> %s\n", CHANNELS[i].gpio, CHANNELS[i].label);
    digitalWrite(CHANNELS[i].gpio, HIGH);
    delay(1500);
    allOff();
    delay(400);
  }

  Serial.println("Test sequence restarting in 3 seconds");
  delay(3000);
}
