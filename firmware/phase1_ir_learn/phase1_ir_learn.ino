/*
 * Phase 1 — IR Signal Learning (full raw dump)
 *
 * Mitsubishi Heavy SRK53MMH1 often shows Protocol: UNKNOWN — this is NORMAL.
 * The built-in decoder does not recognize this 8-byte variant.
 *
 * What to do:
 *   1. Press each remote button once, slowly (wait for "--- End ---" before next)
 *   2. Copy the "Raw send array" block for POWER ON and POWER OFF
 *   3. Paste into firmware/phase2_raw_replay/phase2_raw_replay.ino
 *   4. Also try phase2_ir_transmit (library) — may work on some units
 *
 * Library: IRremoteESP8266 by crankyoldgit
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t kRecvPin = PIN_IR_RECV;
const uint16_t kCaptureBufferSize = 2048;  // large buffer for long AC frames
const uint8_t kTimeout = 120;              // AC remotes send long bursts

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  Serial.println();
  Serial.println("=== Phase 1: IR Signal Learning (full dump) ===");
  Serial.printf("Receiver pin: GPIO %d | Timeout: %u ms\n", kRecvPin, kTimeout);
  Serial.println();
  Serial.println("NOTE: Mitsubishi Heavy SRK series often shows UNKNOWN.");
  Serial.println("      Copy the 'Raw send array' below for phase2_raw_replay.");
  Serial.println("      Press ONE button, wait for output, then next button.");
  Serial.println();

  irrecv.enableIRIn();
}

void loop() {
  static uint32_t lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 8000) {
    lastHeartbeat = millis();
    Serial.printf("[heartbeat %lu ms] waiting for IR...\n", millis());
  }

  if (!irrecv.decode(&results)) {
    return;
  }

  Serial.println();
  Serial.println("========================================");
  Serial.println(resultToHumanReadableBasic(&results).c_str());
  Serial.println("----------------------------------------");
  Serial.printf("Protocol : %s\n", typeToString(results.decode_type).c_str());
  Serial.printf("Value    : 0x%llX\n", results.value);
  Serial.printf("Bits     : %u\n", results.bits);
  Serial.printf("Raw len  : %u\n", results.rawlen);
  if (results.overflow) {
    Serial.println("WARNING  : Buffer overflow — increase kCaptureBufferSize");
  }
  Serial.println("----------------------------------------");

  if (results.decode_type == UNKNOWN) {
    Serial.println(">>> UNKNOWN is expected for SRK53MMH1 — use raw replay >>>");
    Serial.println();
    Serial.println("--- Raw send array (copy this for phase2_raw_replay) ---");
    Serial.println(resultToSourceCode(&results).c_str());
    Serial.println("--- End raw array ---");
  } else {
    Serial.println(">>> Decoded! Protocol name saved for phase2_ir_transmit >>>");
  }

  Serial.println("========================================");
  Serial.println();

  irrecv.resume();
}
