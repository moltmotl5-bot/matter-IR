/*
 * Phase 1 — Custom 8-byte Mitsubishi Heavy SRK decoder
 *
 * Decodes the protocol that IRremoteESP8266 labels "UNKNOWN".
 * Shows: 8-byte frame, power/mode/temp/fan, validation status.
 *
 * Setup:
 *   Copy firmware/libraries/MitsubishiSRK8 to ~/Documents/Arduino/libraries/
 *   (or your Arduino sketchbook libraries folder)
 *
 * Library: IRremoteESP8266 + MitsubishiSRK8 (local)
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <MitsubishiSRK8.h>

const uint16_t kRecvPin = PIN_IR_RECV;
const uint16_t kCaptureBufferSize = 2048;
const uint8_t kTimeout = 120;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  Serial.println();
  Serial.println("=== Phase 1: Mitsubishi SRK 8-byte Decoder ===");
  Serial.printf("Receiver: GPIO %d\n", kRecvPin);
  Serial.println("Press remote buttons. Decoded state appears below.");
  Serial.println();

  irrecv.enableIRIn();
}

void tryDecode() {
  uint8_t frame[8] = {0};
  const bool ok = MitsubishiSRK8::decodeRawBuffer(
      results.rawbuf, results.rawlen, frame, kRawTick);

  Serial.println("--- Custom decode ---");
  MitsubishiSRK8::printFrame(frame);

  if (!ok) {
    Serial.println("Validation: FAILED (header/trailer/checksum mismatch)");
    Serial.println("Tip: press one button slowly; use single 130-bit frame");
    return;
  }

  Serial.println("Validation: OK");

  MitsubishiSRKState state;
  if (MitsubishiSRK8::parseFrame(frame, &state)) {
    MitsubishiSRK8::printState(state);
  }
}

void loop() {
  static uint32_t lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 8000) {
    lastHeartbeat = millis();
    Serial.printf("[heartbeat %lu ms] waiting...\n", millis());
  }

  if (!irrecv.decode(&results)) {
    return;
  }

  Serial.println();
  Serial.println("========================================");
  Serial.printf("Library protocol: %s\n",
                typeToString(results.decode_type).c_str());
  Serial.printf("Raw length      : %u\n", results.rawlen - 1);

  if (results.decode_type == UNKNOWN) {
    tryDecode();
  } else {
    Serial.println("(Named protocol — use IRremoteESP8266 decoder)");
    Serial.println(resultToHumanReadableBasic(&results).c_str());
  }

  Serial.println("========================================");
  Serial.println();

  irrecv.resume();
}
