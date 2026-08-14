/*
 * Phase 2 — Panasonic AC Control
 *
 * Sends Panasonic AC commands via IRremoteESP8266.
 * Point transmitter at AC (VCC on 5V). Serial commands at 115200:
 *
 *   on | off
 *   temp <16-30>
 *   cool | heat | dry | fan | auto
 *   fan auto|low|medium|high|max
 *   quiet on|off | powerful on|off | ion on|off
 *   model unknown|lke|nke|dke|jke|ckp
 *   send | status | help
 *
 * Defaults: protocol ac, model nke (change in config.h if needed)
 */

#include "config.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Panasonic.h>

const uint16_t kIrSendPin = PIN_IR_SEND;
IRPanasonicAc ac(kIrSendPin);

bool useAc32 = DEFAULT_USE_AC32;
uint8_t txBursts = DEFAULT_TX_BURSTS;
uint16_t txBurstGapMs = DEFAULT_TX_BURST_GAP_MS;
uint16_t txRepeat = DEFAULT_TX_REPEAT;

void sendAc() {
  Serial.printf("[TX] Sending %u burst(s), repeat=%u...\n", txBursts, txRepeat);

  for (uint8_t i = 0; i < txBursts; i++) {
    if (useAc32) {
      IRPanasonicAc32 ac32(kIrSendPin);
      ac32.begin();
      ac32.setPowerToggle(ac.getPower());
      ac32.setTemp(ac.getTemp());
      ac32.setMode(ac.getMode());
      ac32.setFan(ac.getFan());
      ac32.send(txRepeat);
    } else {
      ac.send(txRepeat);
    }
    if (i + 1 < txBursts) {
      delay(txBurstGapMs);
    }
  }

  Serial.println("[TX] Done");
  Serial.println(ac.toString().c_str());
}

void printHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  on | off | temp <16-30> | cool heat dry fan auto");
  Serial.println("  fan auto|low|medium|high|max");
  Serial.println("  quiet on|off | powerful on|off | ion on|off");
  Serial.println("  model unknown|lke|nke|dke|jke|ckp|rkr");
  Serial.println("  repeat <1-5> | bursts <1-5>  (increase range/reliability)");
  Serial.println("  protocol ac|ac32   (match Phase 1 result)");
  Serial.println("  send | status | help");
  Serial.println();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  ac.begin();
  ac.setModel(DEFAULT_PANASONIC_MODEL);
  ac.on();
  ac.setMode(kPanasonicAcCool);
  ac.setTemp(24);
  ac.setFan(kPanasonicAcFanAuto);

  Serial.println();
  Serial.println("=== Phase 2: Panasonic AC Control ===");
  Serial.println("Defaults: protocol ac | model nke");
  Serial.printf("Transmitter: GPIO %d | bursts=%u (use 5V for max range)\n",
                kIrSendPin, txBursts);
  printHelp();
}

void handleCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "on") {
    ac.on();
    sendAc();
  } else if (cmd == "off") {
    ac.off();
    sendAc();
  } else if (cmd.startsWith("temp ")) {
    int t = cmd.substring(5).toInt();
    if (t >= 16 && t <= 30) {
      ac.setTemp(static_cast<uint8_t>(t));
      sendAc();
    } else {
      Serial.println("Temp range: 16-30");
    }
  } else if (cmd == "cool") {
    ac.setMode(kPanasonicAcCool);
    sendAc();
  } else if (cmd == "heat") {
    ac.setMode(kPanasonicAcHeat);
    sendAc();
  } else if (cmd == "dry") {
    ac.setMode(kPanasonicAcDry);
    sendAc();
  } else if (cmd == "fan") {
    ac.setMode(kPanasonicAcFan);
    sendAc();
  } else if (cmd == "auto") {
    ac.setMode(kPanasonicAcAuto);
    sendAc();
  } else if (cmd == "fan auto") {
    ac.setFan(kPanasonicAcFanAuto);
    sendAc();
  } else if (cmd == "fan low") {
    ac.setFan(kPanasonicAcFanLow);
    sendAc();
  } else if (cmd == "fan medium") {
    ac.setFan(kPanasonicAcFanMed);
    sendAc();
  } else if (cmd == "fan high") {
    ac.setFan(kPanasonicAcFanMax);
    sendAc();
  } else if (cmd == "fan max") {
    ac.setFan(kPanasonicAcFanMax);
    sendAc();
  } else if (cmd == "quiet on") {
    ac.setQuiet(true);
    sendAc();
  } else if (cmd == "quiet off") {
    ac.setQuiet(false);
    sendAc();
  } else if (cmd == "powerful on") {
    ac.setPowerful(true);
    sendAc();
  } else if (cmd == "powerful off") {
    ac.setPowerful(false);
    sendAc();
  } else if (cmd == "ion on") {
    ac.setIon(true);
    sendAc();
  } else if (cmd == "ion off") {
    ac.setIon(false);
    sendAc();
  } else if (cmd == "model lke") {
    ac.setModel(kPanasonicLke);
    Serial.println("Model: LKE");
  } else if (cmd == "model nke") {
    ac.setModel(kPanasonicNke);
    Serial.println("Model: NKE");
  } else if (cmd == "model dke") {
    ac.setModel(kPanasonicDke);
    Serial.println("Model: DKE");
  } else if (cmd == "model jke") {
    ac.setModel(kPanasonicJke);
    Serial.println("Model: JKE");
  } else if (cmd == "model ckp") {
    ac.setModel(kPanasonicCkp);
    Serial.println("Model: CKP (try protocol ac32 if no response)");
  } else if (cmd == "model rkr") {
    ac.setModel(kPanasonicRkr);
    Serial.println("Model: RKR");
  } else if (cmd == "model unknown") {
    ac.setModel(kPanasonicUnknown);
    Serial.println("Model: auto/unknown");
  } else if (cmd.startsWith("repeat ")) {
    int n = cmd.substring(7).toInt();
    if (n >= 1 && n <= 5) {
      txRepeat = static_cast<uint16_t>(n);
      Serial.printf("TX repeat per burst: %u\n", txRepeat);
    }
  } else if (cmd.startsWith("bursts ")) {
    int n = cmd.substring(7).toInt();
    if (n >= 1 && n <= 5) {
      txBursts = static_cast<uint8_t>(n);
      Serial.printf("TX bursts per command: %u\n", txBursts);
    }
  } else if (cmd == "protocol ac") {
    useAc32 = false;
    Serial.println("Using PANASONIC_AC (IRPanasonicAc)");
  } else if (cmd == "protocol ac32") {
    useAc32 = true;
    Serial.println("Using PANASONIC_AC32 (IRPanasonicAc32)");
  } else if (cmd == "send") {
    sendAc();
  } else if (cmd == "status") {
    Serial.println(ac.toString().c_str());
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
