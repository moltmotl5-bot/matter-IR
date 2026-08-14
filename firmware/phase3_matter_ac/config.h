#pragma once

// --- WiFi (copy secrets.h.example -> secrets.h) ---
#include "secrets.h"

// --- Pins (same as phase 1/2) ---
#define PIN_IR_SEND 15
#define PIN_I2C_SDA 8
#define PIN_I2C_SCL 9

// --- Panasonic IR (confirmed: PANASONIC_AC + NKE, CW-HU series) ---
#define PANASONIC_AC_MODEL kPanasonicNke

// --- AHT20 room sensor (comment out if not wired yet) ---
#define USE_AHT20_SENSOR 1

// --- Matter / AC ---
#define SENSOR_READ_INTERVAL_MS 30000
#define SERIAL_BAUD 115200

// CW-HU window AC typical range
#define AC_MIN_TEMP_C 16
#define AC_MAX_TEMP_C 30
