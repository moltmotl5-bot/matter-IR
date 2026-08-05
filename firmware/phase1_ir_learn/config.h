#pragma once

// ESP32-S3 Matter Smart AC Controller — shared pin & device config
// Wiring (see docs/wiring.md):
//   IR Receiver (HX1838):  VCC→3V3, GND→GND, DAT→GPIO 4
//   IR Transmitter:        VCC→5V,   GND→GND, DAT→GPIO 15
//   AHT20+BMP280 (I2C):    SDA→GPIO 8, SCL→GPIO 9  (phase 3+)

#define PIN_IR_RECV 4
#define PIN_IR_SEND 15

#define PIN_I2C_SDA 8
#define PIN_I2C_SCL 9

#define SERIAL_BAUD 115200
