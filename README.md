# Daily Reminder

Target hardware: ESP32-C3 SuperMini Plus, three common-cathode RGB LEDs, and
one normally-open momentary push button.

## Breadboard wiring

Each of the nine RGB color legs requires its own 220-ohm resistor. Connect each
LED's common-cathode leg to GND.

| Function | GPIO |
|---|---:|
| RGB LED 1 red / green / blue | 0 / 1 / 3 |
| RGB LED 2 red / green / blue | 4 / 5 / 6 |
| RGB LED 3 red / green / blue | 7 / 10 / 20 |
| Silver button | 21 |

Wire the normally-open button between GPIO21 and GND. The firmware uses the
ESP32-C3's internal pull-up resistor.

GPIO2, GPIO8, and GPIO9 are intentionally unused because they are ESP32-C3
strapping pins. GPIO18 and GPIO19 are reserved for native USB-JTAG.

GPIO20 and GPIO21 are also the ESP32-C3's default UART pins. Because this build
uses them for LED 3 blue and the button, production firmware deliberately does
not initialize `Serial`; doing so can flash GPIO20 and interfere with GPIO21.

## Arduino IDE

1. Install Espressif's `esp32` board package.
2. Open `PillReminderC3.ino`.
3. Select an ESP32-C3 board profile compatible with the SuperMini.
4. Enable **USB CDC On Boot** if that option is available.
5. Edit `PillReminderConfig.h` with Wi-Fi credentials.
6. Compile and upload.

The sketch uses only libraries included with the Arduino ESP32 core.

## Current release candidate (v0.3.0)

Before confirmation, `PillReminderC3.ino` runs an eight-second attention loop containing a
six-color chase, red/blue warning lights, mixed-color rotation, white strobe,
and a comet chase. Pressing and releasing the button changes all three LEDs to
steady green at approximately 12% duty, which should look clearly dimmer than
the attention pattern.

- A medication day runs from 2:00 a.m. until 1:59:59 a.m. the next day.
- A short button press stores the current medication day in flash and changes
  the LEDs to dim green.
- The device uses NTP plus the Chicago time-zone rule, including daylight
  saving time, to clear the confirmation at 2:00 a.m.
- After a restart, the device obtains Chicago time and restores the saved state
  for the current medication day.
- Optional Google Sheet logging sends a TAKEN event when configured.
- Serial remains disabled because GPIO20 and GPIO21 are used by the project.

If Wi-Fi is unavailable, the light and button still work. A confirmation made
before time synchronization is attached to the current medication day as soon
as NTP becomes available.

## Google Sheet logging

Logging is disabled while `WEBHOOK_URL` remains the placeholder in
`PillReminderConfig.h`.

1. Open the existing Pill Reminder log spreadsheet.
2. Open **Extensions > Apps Script**.
3. Paste `GoogleAppsScript.gs` into the editor.
4. Change `DEVICE_TOKEN` in both files to the same private random phrase.
5. Confirm that `LOG_SHEET_NAME` matches the log tab's visible name.
6. Deploy the script as a Web app and copy its `/exec` URL into
   `PillReminderConfig.h`.

The first breadboard test can be completed before setting up logging.
