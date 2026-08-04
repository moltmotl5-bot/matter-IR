/*
 * Phase 2 — IR Transmit Test (Mitsubishi Heavy AC)
 *
 * Sends "Power ON, Cool mode, 24°C" every 10 seconds.
 * Point IR transmitter at AC (~3m) and verify the unit responds.
 *
 * If Phase 1 showed MitsubishiHeavy88 instead of 152, change the class below.
 *
 * Library: IRremoteESP8266 by crankyoldgit
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <ir_MitsubishiHeavy.h>

const uint16_t kIrSendPin = PIN_IR_SEND;
const uint32_t kSendIntervalMs = 10000;

// Change to IRMitsubishiHeavy88Ac if Phase 1 detected MitsubishiHeavy88
IRMitsubishiHeavy152Ac ac(kIrSendPin);

void setup() {
  Serial.begin(SERIAL_BAUD);
#if defined(ARDUINO_ESP32S3_DEV) || defined(CONFIG_IDF_TARGET_ESP32S3)
  while (!Serial && millis() < 3000) {
    delay(10);
  }
#endif

  Serial.println();
  Serial.println("=== Phase 2: IR Transmit Test ===");
  Serial.printf("Transmitter pin: GPIO %d\n", kIrSendPin);
  Serial.println("Sending: ON, Cool, 24°C every 10 seconds");
  Serial.println("Point transmitter at AC and watch for response.");
  Serial.println();

  ac.begin();
}

void sendAcCommand() {
  ac.on();
  ac.setMode(kMitsubishiHeavyCool);
  ac.setTemp(24);
  ac.setFan(kMitsubishiHeavyFanAuto);
  ac.send();

  Serial.println("[TX] Sent: Power ON, Cool, 24°C, Fan Auto");
}

void loop() {
  sendAcCommand();
  delay(kSendIntervalMs);
}
