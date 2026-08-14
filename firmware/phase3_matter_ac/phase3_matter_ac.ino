/*
 * Phase 3 — Matter Thermostat + Panasonic IR (CW-HU series)
 *
 * Apple Home controls a Matter Thermostat endpoint; changes trigger IR to the AC.
 * Confirmed IR: PANASONIC_AC, model NKE (same as phase 2).
 *
 * Setup:
 *   1. Copy secrets.h.example -> secrets.h, set WiFi SSID/password
 *   2. Arduino IDE: esp32 3.0+, Partition 16M Flash (3MB APP/9.9MB FATFS)
 *   3. Libraries: IRremoteESP8266, Adafruit AHTX0 (+ dependencies) if USE_AHT20_SENSOR
 *   4. Upload, open Serial 115200, pair via QR code in Apple Home
 *
 * Decommission: hold BOOT button 5 seconds
 */

#include "config.h"
#include <Arduino.h>
#include <Matter.h>
#include <WiFi.h>
#include <IRremoteESP8266.h>
#include <ir_Panasonic.h>

#if USE_AHT20_SENSOR
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#endif

MatterThermostat matterThermostat;
IRPanasonicAc panasonicAc(PIN_IR_SEND);

#if USE_AHT20_SENSOR
Adafruit_AHTX0 aht20;
bool aht20Ok = false;
#endif

const uint8_t kButtonPin = BOOT_PIN;
uint32_t buttonTimestamp = 0;
bool buttonPressed = false;
const uint32_t kDecommissionMs = 5000;

uint32_t lastSensorReadMs = 0;
bool irInitialized = false;

uint8_t clampAcTemp(double celsius) {
  int t = static_cast<int>(celsius + 0.5);
  if (t < AC_MIN_TEMP_C) {
    t = AC_MIN_TEMP_C;
  }
  if (t > AC_MAX_TEMP_C) {
    t = AC_MAX_TEMP_C;
  }
  return static_cast<uint8_t>(t);
}

void sendPanasonicIr() {
  if (!irInitialized) {
    return;
  }
  panasonicAc.send();
  Serial.println("[IR] TX sent");
  Serial.println(panasonicAc.toString().c_str());
}

void applyMatterToPanasonicAc() {
  if (!Matter.isDeviceCommissioned()) {
    return;
  }

  const auto mode = matterThermostat.getMode();
  Serial.printf("[AC] Matter mode: %s\n",
                MatterThermostat::getThermostatModeString(mode));

  switch (mode) {
    case MatterThermostat::THERMOSTAT_MODE_OFF:
      panasonicAc.off();
      break;

    case MatterThermostat::THERMOSTAT_MODE_COOL:
    case MatterThermostat::THERMOSTAT_MODE_AUTO:
      panasonicAc.on();
      panasonicAc.setMode(kPanasonicAcCool);
      panasonicAc.setTemp(clampAcTemp(matterThermostat.getCoolingSetpoint()));
      panasonicAc.setFan(kPanasonicAcFanAuto);
      break;

    case MatterThermostat::THERMOSTAT_MODE_HEAT:
      panasonicAc.on();
      panasonicAc.setMode(kPanasonicAcHeat);
      panasonicAc.setTemp(clampAcTemp(matterThermostat.getHeatingSetpoint()));
      panasonicAc.setFan(kPanasonicAcFanAuto);
      break;

    case MatterThermostat::THERMOSTAT_MODE_DRY:
      panasonicAc.on();
      panasonicAc.setMode(kPanasonicAcDry);
      panasonicAc.setTemp(clampAcTemp(matterThermostat.getCoolingSetpoint()));
      break;

    case MatterThermostat::THERMOSTAT_MODE_FAN_ONLY:
      panasonicAc.on();
      panasonicAc.setMode(kPanasonicAcFan);
      break;

    default:
      Serial.printf("[AC] Mode %u not mapped — no IR sent\n", mode);
      return;
  }

  sendPanasonicIr();
}

bool onMatterThermostatChange() {
  applyMatterToPanasonicAc();
  return true;
}

void initPanasonicIr() {
  panasonicAc.begin();
  panasonicAc.setModel(PANASONIC_AC_MODEL);
  panasonicAc.off();
  panasonicAc.setMode(kPanasonicAcCool);
  panasonicAc.setTemp(24);
  panasonicAc.setFan(kPanasonicAcFanAuto);
  irInitialized = true;
  Serial.println("[IR] Panasonic AC ready (PANASONIC_AC, model NKE)");
}

#if USE_AHT20_SENSOR
bool initAht20() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (!aht20.begin()) {
    Serial.println("[SENSOR] AHT20 not found — using Matter temp only");
    return false;
  }
  Serial.println("[SENSOR] AHT20 OK");
  return true;
}

void updateRoomTemperature() {
  if (!aht20Ok) {
    return;
  }
  sensors_event_t humidity, temp;
  if (!aht20.getEvent(&humidity, &temp)) {
    return;
  }
  const double roomC = temp.temperature;
  matterThermostat.setLocalTemperature(roomC);
  Serial.printf("[SENSOR] Room %.1f C, humidity %.1f %%\n", roomC,
                humidity.relative_humidity);
}
#else
bool initAht20() {
  return false;
}
void updateRoomTemperature() {}
#endif

void connectWifi() {
  Serial.printf("[WiFi] Connecting to %s ...\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("[WiFi] Connected, IP: %s\n", WiFi.localIP().toString().c_str());
}

void waitForCommissioning() {
  if (Matter.isDeviceCommissioned()) {
    return;
  }

  Serial.println();
  Serial.println("=== Matter pairing ===");
  Serial.println("Apple Home: Add Accessory -> scan QR or enter manual code");
  Serial.printf("Manual code: %s\n", Matter.getManualPairingCode().c_str());
  Serial.printf("QR URL: %s\n", Matter.getOnboardingQRCodeUrl().c_str());

  uint32_t count = 0;
  while (!Matter.isDeviceCommissioned()) {
    delay(100);
    if ((++count % 50) == 0) {
      Serial.println("[Matter] Waiting for commissioning...");
    }
  }
  Serial.println("[Matter] Commissioned!");
}

void setup() {
  pinMode(kButtonPin, INPUT_PULLUP);
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  Serial.println();
  Serial.println("=== Phase 3: Matter Thermostat + Panasonic CW-HU ===");

  initPanasonicIr();
#if USE_AHT20_SENSOR
  aht20Ok = initAht20();
#endif

  connectWifi();

  // CW-HU window AC: cooling-focused thermostat (no AUTO heat/cool loop)
  matterThermostat.onChange(onMatterThermostatChange);
  matterThermostat.begin(MatterThermostat::THERMOSTAT_SEQ_OP_COOLING,
                         MatterThermostat::THERMOSTAT_AUTO_MODE_DISABLED);

  Matter.begin();
  waitForCommissioning();

  // Initial Home state
  matterThermostat.setCoolingSetpoint(24.0);
  matterThermostat.setLocalTemperature(25.0);
  matterThermostat.setMode(MatterThermostat::THERMOSTAT_MODE_OFF);

#if USE_AHT20_SENSOR
  updateRoomTemperature();
#endif

  Serial.println();
  Serial.println("Ready. Control from Apple Home — IR sends on change.");
  Serial.println("Hold BOOT 5s to decommission.");
}

void handleDecommissionButton() {
  if (digitalRead(kButtonPin) == LOW && !buttonPressed) {
    buttonTimestamp = millis();
    buttonPressed = true;
  }
  if (digitalRead(kButtonPin) == HIGH && buttonPressed) {
    buttonPressed = false;
  }
  if (buttonPressed && (millis() - buttonTimestamp > kDecommissionMs)) {
    Serial.println("[Matter] Decommissioning...");
    Matter.decommission();
    buttonTimestamp = millis();
  }
}

void loop() {
  handleDecommissionButton();

  if (millis() - lastSensorReadMs >= SENSOR_READ_INTERVAL_MS) {
    lastSensorReadMs = millis();
    updateRoomTemperature();
  }

  delay(200);
}
