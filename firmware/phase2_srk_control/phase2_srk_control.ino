/*
 * Phase 2 — Mitsubishi SRK encoder + transmit
 *
 * Uses decoded 8-byte protocol to control AC directly.
 * No raw replay needed — builds correct frames from state.
 *
 * Serial commands (115200):
 *   on              power on (keeps current mode/temp/fan)
 *   off             power off
 *   temp <16-31>    set temperature
 *   cool|heat|dry|fan|auto   set mode
 *   fan low|med|high         set fan speed
 *   status          print current state
 *   help
 *
 * Copy firmware/libraries/MitsubishiSRK8 to Arduino libraries folder.
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <MitsubishiSRK8.h>

const uint16_t kIrSendPin = PIN_IR_SEND;
IRsend irsend(kIrSendPin);

MitsubishiSRKState acState;
uint16_t rawBuf[140];

void sendState() {
  const size_t len = MitsubishiSRK8::encodeState(acState, rawBuf, 140);
  if (len == 0) {
    Serial.println("[ERR] encode failed");
    return;
  }

  Serial.println("[TX] Sending:");
  MitsubishiSRK8::printState(acState);

  for (uint8_t i = 0; i < MitsubishiSRKTiming::kRepeatCount; i++) {
    irsend.sendRaw(rawBuf, len, MitsubishiSRKTiming::kCarrierHz);
    delay(MitsubishiSRKTiming::kRepeatGapMs);
  }
  Serial.println("[TX] Done (3 repeats)");
}

void printHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  on | off | temp <16-31> | cool heat dry fan auto");
  Serial.println("  fan low|med|high | status | help");
  Serial.println();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  irsend.begin();

  acState.power = true;
  acState.mode = kSRKModeCool;
  acState.temp_c = 24;
  acState.fan = kSRKFanMedium;

  Serial.println();
  Serial.println("=== Phase 2: Mitsubishi SRK Control ===");
  Serial.printf("Transmitter: GPIO %d\n", kIrSendPin);
  Serial.println("Point IR LED at AC. Type 'help' for commands.");
  printHelp();
}

void handleCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "on") {
    acState.power = true;
    sendState();
  } else if (cmd == "off") {
    acState.power = false;
    sendState();
  } else if (cmd.startsWith("temp ")) {
    int t = cmd.substring(5).toInt();
    if (t >= 16 && t <= 31) {
      acState.temp_c = static_cast<uint8_t>(t);
      sendState();
    } else {
      Serial.println("Temp range: 16-31");
    }
  } else if (cmd == "cool") {
    acState.mode = kSRKModeCool;
    sendState();
  } else if (cmd == "heat") {
    acState.mode = kSRKModeHeat;
    sendState();
  } else if (cmd == "dry") {
    acState.mode = kSRKModeDry;
    sendState();
  } else if (cmd == "fan") {
    acState.mode = kSRKModeFan;
    sendState();
  } else if (cmd == "auto") {
    acState.mode = kSRKModeAuto;
    sendState();
  } else if (cmd == "fan low") {
    acState.fan = kSRKFanLow;
    sendState();
  } else if (cmd == "fan med" || cmd == "fan medium") {
    acState.fan = kSRKFanMedium;
    sendState();
  } else if (cmd == "fan high") {
    acState.fan = kSRKFanHigh;
    sendState();
  } else if (cmd == "status") {
    MitsubishiSRK8::printState(acState);
  } else if (cmd == "help") {
    printHelp();
  } else if (cmd.length() > 0) {
    Serial.printf("Unknown: %s\n", cmd.c_str());
    printHelp();
  }
}

void loop() {
  if (!Serial.available()) {
    return;
  }
  handleCommand(Serial.readStringUntil('\n'));
}
