# Daily Reminder

An ESP32-C3 medication reminder with three RGB LEDs and a confirmation button.

## Current status

Firmware v0.2.0 is the light-show checkpoint. It deliberately excludes Wi-Fi,
timekeeping, storage, logging, and Serial while the display behavior is tuned.

Before confirmation, the eight-second attention loop contains:

- Six-color chase
- Alternating red/blue warning lights
- Yellow/cyan/magenta rotation
- White strobe
- Red/green/blue comet chase

Pressing and releasing the button changes all three LEDs to green at
approximately 50% brightness. Rebooting restarts the attention state.

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

## Configuration and secrets

Never commit `PillReminderConfig.h`; it may contain a Wi-Fi password and webhook
credentials. Copy `PillReminderConfig.example.h` to `PillReminderConfig.h`
locally when full Wi-Fi functionality is restored.

## Planned behavior

- Attention animation until medication is confirmed
- Button press changes the display to dim steady green
- Medication day resets at 2:00 a.m. Chicago time
- State survives power loss
- Optional Google Sheets event logging
