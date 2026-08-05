/*
 * Phase 2 — Raw IR Replay (for UNKNOWN Mitsubishi Heavy remotes)
 *
 * SRK53MMH1 uses an 8-byte variant that IRremoteESP8266 cannot decode.
 * This sketch replays the EXACT timings captured in Phase 1.
 *
 * SETUP:
 *   1. Re-flash phase1_ir_learn.ino
 *   2. Press remote POWER ON — copy the full "Raw send array" from Serial Monitor
 *   3. Replace POWER_ON_RAW and POWER_ON_LEN below with your captured data
 *   4. Repeat for POWER OFF (optional: temp up/down)
 *   5. Upload this sketch and point IR transmitter at AC (~3 m)
 *
 * Serial commands (115200):
 *   1 = send POWER ON raw
 *   0 = send POWER OFF raw
 *   u = send TEMP UP raw (if captured)
 *   d = send TEMP DOWN raw (if captured)
 *   help
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>

const uint16_t kIrSendPin = PIN_IR_SEND;
IRsend irsend(kIrSendPin);

// ── Paste from Phase 1 Serial output ─────────────────────────────────────
// Example placeholder (REPLACE with your captured arrays):
// Power ON from your capture: Value 0xA6D40490, Bits 130

uint16_t POWER_ON_RAW[] = {
  // TODO: paste numbers from Phase 1 "Raw send array" for POWER ON
  // e.g. 5950, 7450, 508, 1500, 508, 3450, ...
  0
};
const uint16_t POWER_ON_LEN = 0;  // TODO: set to array length from Phase 1

uint16_t POWER_OFF_RAW[] = {
  // TODO: paste for POWER OFF (use the 130-bit capture, not the 260-bit double frame)
  0
};
const uint16_t POWER_OFF_LEN = 0;

uint16_t TEMP_UP_RAW[] = { 0 };
const uint16_t TEMP_UP_LEN = 0;

uint16_t TEMP_DOWN_RAW[] = { 0 };
const uint16_t TEMP_DOWN_LEN = 0;

// ───────────────────────────────────────────────────────────────────────────

void sendRaw(const char* label, uint16_t* raw, uint16_t len) {
  if (len < 2 || raw[0] == 0) {
    Serial.printf("[SKIP] %s — paste raw array from Phase 1 first (len=%u)\n", label, len);
    return;
  }
  irsend.sendRaw(raw, len, IR_CARRIER_HZ);
  Serial.printf("[TX] %s sent (%u timings)\n", label, len);
}

void printHelp() {
  Serial.println();
  Serial.println("Commands: 1=power on | 0=power off | u=temp up | d=temp down | help");
  Serial.println();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  irsend.begin();

  Serial.println();
  Serial.println("=== Phase 2: Raw IR Replay ===");
  Serial.printf("Transmitter pin: GPIO %d\n", kIrSendPin);
  Serial.println("Paste Phase 1 raw arrays into this file, then re-upload.");
  printHelp();

  if (POWER_ON_LEN > 1) {
    Serial.println("POWER ON raw loaded — press '1' to test");
  } else {
    Serial.println("WARNING: POWER_ON_RAW not configured yet");
  }
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  char c = Serial.read();
  switch (c) {
    case '1':
      sendRaw("POWER ON", POWER_ON_RAW, POWER_ON_LEN);
      break;
    case '0':
      sendRaw("POWER OFF", POWER_OFF_RAW, POWER_OFF_LEN);
      break;
    case 'u':
      sendRaw("TEMP UP", TEMP_UP_RAW, TEMP_UP_LEN);
      break;
    case 'd':
      sendRaw("TEMP DOWN", TEMP_DOWN_RAW, TEMP_DOWN_LEN);
      break;
    case 'h':
    case '?':
      printHelp();
      break;
    default:
      break;
  }
}
