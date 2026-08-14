/*
 * Phase 1 — Panasonic AC IR Learning
 *
 * Point your Panasonic remote at the IR receiver and press buttons.
 * IRremoteESP8266 usually decodes as PANASONIC_AC or PANASONIC_AC32.
 *
 * Library: IRremoteESP8266 (crankyoldgit)
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ir_Panasonic.h>

const uint16_t kRecvPin = PIN_IR_RECV;
const uint16_t kCaptureBufferSize = 4096;  // Panasonic AC frames can be long
const uint8_t kTimeout = 150;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void decodePanasonicAc(const decode_results& r) {
  IRPanasonicAc ac(PIN_IR_SEND);
  ac.setRaw(r.state);
  ac.setModel(ac.getModel());  // auto-detect model from frame

  Serial.println("--- Panasonic AC decode ---");
  Serial.println(ac.toString().c_str());
  Serial.printf("Power : %s\n", ac.getPower() ? "ON" : "OFF");
  Serial.printf("Mode  : %u\n", ac.getMode());
  Serial.printf("Temp  : %u C\n", ac.getTemp());
  Serial.printf("Fan   : %u\n", ac.getFan());
  Serial.printf("Model : %u\n", static_cast<unsigned>(ac.getModel()));
  Serial.println(">>> Save protocol name PANASONIC_AC for Phase 2 <<<");
}

void decodePanasonicAc32(const decode_results& r) {
  IRPanasonicAc32 ac32(PIN_IR_SEND);
  ac32.setRaw(r.value);

  Serial.println("--- Panasonic AC32 decode ---");
  Serial.println(ac32.toString().c_str());
  Serial.printf("Power : %s\n", ac32.getPowerToggle() ? "ON" : "OFF");
  Serial.printf("Temp  : %u C\n", ac32.getTemp());
  Serial.println(">>> Save protocol name PANASONIC_AC32 for Phase 2 <<<");
}

void dumpUnknown(const decode_results& r) {
  Serial.println("--- UNKNOWN — dump raw (paste if needed) ---");
  Serial.println(resultToHumanReadableBasic(&r).c_str());
  Serial.println(resultToSourceCode(&r).c_str());
  Serial.println("Try Phase 2 anyway — some Panasonic units still respond.");
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  Serial.println();
  Serial.println("=== Phase 1: Panasonic AC IR Learning ===");
  Serial.printf("Receiver: GPIO %d | Timeout: %u ms\n", kRecvPin, kTimeout);
  Serial.println("Press Panasonic remote buttons (power, temp, mode).");
  Serial.println();

  irrecv.enableIRIn();
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
  Serial.printf("Protocol : %s\n", typeToString(results.decode_type).c_str());
  Serial.printf("Bits     : %u\n", results.bits);
  Serial.printf("Value    : 0x%llX\n", results.value);
  Serial.printf("Raw len  : %u\n", results.rawlen);

  switch (results.decode_type) {
    case PANASONIC_AC:
      decodePanasonicAc(results);
      break;
    case PANASONIC_AC32:
      decodePanasonicAc32(results);
      break;
    case UNKNOWN:
      dumpUnknown(results);
      break;
    default:
      Serial.println("(Other protocol — confirm this is a Panasonic remote)");
      Serial.println(resultToHumanReadableBasic(&results).c_str());
      break;
  }

  Serial.println("========================================");
  Serial.println();
  irrecv.resume();
}
