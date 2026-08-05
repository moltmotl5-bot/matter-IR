/*
 * Phase 1 — IR Signal Learning (IRrecvDumpV3)
 *
 * Connect ESP32-S3 to Mac via USB-C (COM/UART port).
 * Open Serial Monitor at 115200 baud.
 * Point your Mitsubishi Heavy remote at the IR receiver and press buttons.
 * Record the Protocol name (e.g. MitsubishiHeavy152) from serial output.
 *
 * Library: IRremoteESP8266 by crankyoldgit
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t kRecvPin = PIN_IR_RECV;
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 50;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);  // allow Mac USB to enumerate before first print

  Serial.println();
  Serial.println("=== Phase 1: IR Signal Learning ===");
  Serial.printf("Receiver pin: GPIO %d\n", kRecvPin);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.println("Serial: USB CDC (connect to native USB port)");
#else
  Serial.println("Serial: UART/CH343 (connect to COM/UART port)");
#endif
  Serial.println("Point Mitsubishi Heavy remote at receiver, press buttons.");
  Serial.println("Heartbeat prints every 5s if no IR signal yet.");
  Serial.println();

  irrecv.enableIRIn();
}

void loop() {
  static uint32_t lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 5000) {
    lastHeartbeat = millis();
    Serial.printf("[heartbeat %lu ms] waiting for IR...\n", millis());
  }

  if (irrecv.decode(&results)) {
    Serial.println();
    Serial.println("--- IR Signal Captured ---");
    Serial.printf("Protocol: %s\n", typeToString(results.decode_type).c_str());
    Serial.printf("Value:    0x%llX\n", results.value);
    Serial.printf("Bits:     %u\n", results.bits);
    Serial.printf("Raw len:  %u\n", results.rawlen);

    if (results.decode_type != UNKNOWN) {
      Serial.println();
      Serial.println(">>> SAVE THIS PROTOCOL NAME for Phase 2 <<<");
      Serial.printf(">>> Expected: MitsubishiHeavy152 or MitsubishiHeavy88 <<<\n");
    }

    Serial.println("--- End ---");
    Serial.println();

    irrecv.resume();
  }
}
