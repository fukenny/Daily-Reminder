# Daily Reminder

An ESP32-C3 medication reminder with three RGB LEDs and a confirmation button.

## Current status

Firmware v0.1.4 is a hardware bring-up build. It deliberately excludes Wi-Fi,
timekeeping, storage, logging, Serial, and PWM while the integrated LED and
button behavior is verified.

The current sequence lights exactly one channel at a time:

1. LED 1: red, green, blue
2. LED 2: red, green, blue
3. LED 3: red, green, blue

Each illuminated step lasts 900 ms followed by a 300 ms dark interval. Pressing
and releasing the button toggles between the test sequence and all three LEDs
solid green.

## Hardware

- ESP32-C3 SuperMini Plus
- Three 5 mm common-cathode RGB LEDs
- Nine 220-ohm resistors
- Normally-open momentary button
- USB power

| Function | GPIO |
|---|---:|
| LED 1 red / green / blue | 0 / 1 / 3 |
| LED 2 red / green / blue | 4 / 5 / 6 |
| LED 3 red / green / blue | 7 / 10 / 20 |
| Button to GND | 21 |

Each RGB color leg has its own 220-ohm resistor. Every common-cathode leg and
the button ground share GND.

## Repository layout

- `firmware/PillReminderC3/`: current integrated firmware
- `diagnostics/RGBHardwareTest/`: isolated nine-channel GPIO test
- `google-apps-script/`: optional Google Sheets logging endpoint
- `docs/`: project documentation

## Configuration and secrets

Never commit `PillReminderConfig.h`; it may contain a Wi-Fi password and webhook
credentials. Copy `PillReminderConfig.example.h` to `PillReminderConfig.h`
locally when full Wi-Fi functionality is restored.

## Planned behavior

- Attention animation until medication is confirmed
- Button press changes the display to steady green
- Medication day resets at 2:00 a.m. Chicago time
- State survives power loss
- Optional Google Sheets event logging
