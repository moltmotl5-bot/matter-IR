/*
 * Phase 2b — Interactive IR Transmit Test
 *
 * Serial commands (115200 baud):
 *   on       — power on, cool 24°C
 *   off      — power off
 *   temp NN  — set temperature (16-31)
 *   cool     — cooling mode
 *   heat     — heating mode
 *   dry      — dry mode
 *   fan      — fan only mode
 *   send     — re-send current state
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <ir_MitsubishiHeavy.h>

const uint16_t kIrSendPin = PIN_IR_SEND;

IRMitsubishiHeavy152Ac ac(kIrSendPin);

void sendState() {
  ac.send();
  Serial.println("[TX] State sent.");
}

void printHelp() {
  Serial.println();
  Serial.println("Commands: on | off | temp <16-31> | cool | heat | dry | fan | send | help");
  Serial.println();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
#if defined(ARDUINO_ESP32S3_DEV) || defined(CONFIG_IDF_TARGET_ESP32S3)
  while (!Serial && millis() < 3000) {
    delay(10);
  }
#endif

  ac.begin();
  ac.on();
  ac.setMode(kMitsubishiHeavyCool);
  ac.setTemp(24);
  ac.setFan(kMitsubishiHeavyFanAuto);

  Serial.println();
  Serial.println("=== Phase 2b: Interactive IR Transmit ===");
  Serial.printf("Transmitter pin: GPIO %d\n", kIrSendPin);
  printHelp();
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "on") {
    ac.on();
    ac.setMode(kMitsubishiHeavyCool);
    ac.setTemp(24);
    sendState();
  } else if (cmd == "off") {
    ac.off();
    sendState();
  } else if (cmd.startsWith("temp ")) {
    int temp = cmd.substring(5).toInt();
    if (temp >= 16 && temp <= 31) {
      ac.setTemp(temp);
      Serial.printf("Temperature set to %d°C\n", temp);
      sendState();
    } else {
      Serial.println("Invalid temp. Use 16-31.");
    }
  } else if (cmd == "cool") {
    ac.setMode(kMitsubishiHeavyCool);
    sendState();
  } else if (cmd == "heat") {
    ac.setMode(kMitsubishiHeavyHeat);
    sendState();
  } else if (cmd == "dry") {
    ac.setMode(kMitsubishiHeavyDry);
    sendState();
  } else if (cmd == "fan") {
    ac.setMode(kMitsubishiHeavyFan);
    sendState();
  } else if (cmd == "send") {
    sendState();
  } else if (cmd == "help") {
    printHelp();
  } else {
    Serial.printf("Unknown command: %s\n", cmd.c_str());
    printHelp();
  }
}
