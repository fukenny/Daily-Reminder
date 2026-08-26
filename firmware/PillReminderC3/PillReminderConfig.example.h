#pragma once  // Project-specific name avoids Arduino library-header collisions.

// Fill these in before uploading. The lights and button still work without Wi-Fi,
// but automatic 2:00 a.m. reset requires network time.
constexpr char WIFI_SSID[] = "YOUR_WIFI_NAME";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";

// Paste the deployed Google Apps Script /exec URL here when it is available.
// Leave the placeholder unchanged to disable remote logging during early tests.
constexpr char WEBHOOK_URL[] = "PASTE_GOOGLE_APPS_SCRIPT_EXEC_URL_HERE";

// This must match DEVICE_TOKEN in GoogleAppsScript.gs. Change both copies to a
// private random phrase before deploying the web app.
constexpr char DEVICE_TOKEN[] = "CHANGE_THIS_TO_A_PRIVATE_RANDOM_PHRASE";
constexpr char DEVICE_NAME[] = "Pill Reminder v1";
constexpr char FIRMWARE_VERSION[] = "v0.3.0";

// Chicago time, including automatic daylight-saving transitions.
constexpr char TIME_ZONE[] = "CST6CDT,M3.2.0/2,M11.1.0/2";
constexpr char NTP_SERVER_1[] = "pool.ntp.org";
constexpr char NTP_SERVER_2[] = "time.nist.gov";

constexpr uint8_t RESET_HOUR = 2;
constexpr uint8_t CONFIRMED_GREEN_BRIGHTNESS = 12;  // Percent, 1-100
