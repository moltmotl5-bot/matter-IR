/*
 * Phase 1 — Panasonic AC IR Learning
 *
 * If Serial Monitor is blank:
 *   1. Upload firmware/serial_test/serial_test.ino first (USB/CDC pairing)
 *   2. Match USB port + "USB CDC On Boot" — see docs/troubleshooting-serial-mac.md
 *   3. Baud 115200, press EN/RST after upload, then open Serial Monitor
 *
 * Library: IRremoteESP8266 (crankyoldgit)
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ir_Panasonic.h>

const uint16_t kRecvPin = PIN_IR_RECV;
const uint16_t kCaptureBufferSize = 2048;
const uint8_t kTimeout = 150;

IRrecv* irrecv = nullptr;
decode_results results;

void decodePanasonicAc(const decode_results& r) {
  IRPanasonicAc ac(PIN_IR_SEND);
  ac.setRaw(r.state);
  ac.setModel(ac.getModel());

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
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  Serial.println();
  Serial.println("=== Phase 1: Panasonic AC IR Learning ===");
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.println("Serial: USB CDC ENABLED  -> use native USB port");
#else
  Serial.println("Serial: USB CDC DISABLED -> use COM/UART port");
#endif
  Serial.printf("Receiver: GPIO %d | Timeout: %u ms\n", kRecvPin, kTimeout);
  Serial.println("Waiting for IR... heartbeat every 3s.");
  Serial.println("Press Panasonic remote: power, temp up/down.");
  Serial.flush();

  irrecv = new IRrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
  if (irrecv == nullptr) {
    Serial.println("ERROR: IR receiver init failed (out of memory?)");
    return;
  }
  irrecv->enableIRIn();
  Serial.println("IR receiver ready.");
  Serial.flush();
}

void loop() {
  static uint32_t lastHeartbeat = 0;

  if (irrecv == nullptr) {
    delay(1000);
    return;
  }

  if (millis() - lastHeartbeat >= 3000) {
    lastHeartbeat = millis();
    Serial.printf("[heartbeat %lu ms] waiting for remote...\n", millis());
    Serial.flush();
  }

  if (!irrecv->decode(&results)) {
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
      Serial.println("(Other protocol — confirm Panasonic remote)");
      Serial.println(resultToHumanReadableBasic(&results).c_str());
      break;
  }

  Serial.println("========================================");
  Serial.println();
  Serial.flush();
  irrecv->resume();
}
