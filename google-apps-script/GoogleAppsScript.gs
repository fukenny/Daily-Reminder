// Attach this script to the "Pill Reminder - Button Press Log" spreadsheet.
// Deploy it as a Web app, executing as yourself. Use the resulting /exec URL
// as WEBHOOK_URL in PillReminderConfig.h.

const DEVICE_TOKEN = 'CHANGE_THIS_TO_A_PRIVATE_RANDOM_PHRASE';
const LOG_SHEET_NAME = 'Pill Log';

function doPost(e) {
  try {
    const data = JSON.parse(e.postData.contents || '{}');
    if (data.token !== DEVICE_TOKEN) {
      return jsonResponse({ok: false, error: 'unauthorized'});
    }

    const sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(LOG_SHEET_NAME);
    if (!sheet) {
      return jsonResponse({ok: false, error: `Missing sheet: ${LOG_SHEET_NAME}`});
    }

    const epoch = Number(data.epoch || 0);
    const eventTime = epoch > 0 ? new Date(epoch * 1000) : new Date();
    sheet.appendRow([
      eventTime,
      String(data.cycle || ''),
      String(data.event || '').toUpperCase(),
      String(data.device || ''),
      String(data.firmware || ''),
      'NTP / Wi-Fi',
      `Received by web app ${new Date().toISOString()}`
    ]);

    return jsonResponse({ok: true});
  } catch (error) {
    return jsonResponse({ok: false, error: String(error)});
  }
}

function jsonResponse(payload) {
  return ContentService
    .createTextOutput(JSON.stringify(payload))
    .setMimeType(ContentService.MimeType.JSON);
}
