/*
 * Serial Port Diagnostic — upload this FIRST if Serial Monitor shows nothing.
 *
 * Expected: a line every second like "[12345 ms] Serial OK — count=3"
 *
 * Arduino IDE board settings (try BOTH combinations below if needed):
 *
 *   Option A (native USB port, usually labeled "USB"):
 *     USB CDC On Boot = Enabled
 *     Connect cable to the USB port (not COM/UART)
 *
 *   Option B (CH343 UART port, usually labeled "COM" or "UART"):
 *     USB CDC On Boot = Disabled
 *     Connect cable to the COM/UART port
 *     Install CH343 Mac driver if port is missing
 *
 * Serial Monitor: 115200 baud, set line ending to "Newline" or "Both NL & CR"
 * After upload: press the EN/RST button once, then open Serial Monitor
 */

#include "config.h"

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);  // allow USB enumeration on Mac

  Serial.println();
  Serial.println("========================================");
  Serial.println("  ESP32-S3 Serial Test — setup() ran OK");
  Serial.println("========================================");
  Serial.printf("Baud: %d\n", SERIAL_BAUD);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.println("USB CDC On Boot: ENABLED  (use native USB port)");
#else
  Serial.println("USB CDC On Boot: DISABLED (use COM/UART + CH343 port)");
#endif
  Serial.println("You should see a line every second below.");
  Serial.println();
}

void loop() {
  static uint32_t count = 0;
  count++;
  Serial.printf("[%lu ms] Serial OK — count=%lu\n", millis(), count);
  delay(1000);
}
