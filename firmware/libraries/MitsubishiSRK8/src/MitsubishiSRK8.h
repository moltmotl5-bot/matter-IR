#pragma once

#include <stdint.h>
#include <stddef.h>

// Mitsubishi Heavy SRK-series 8-byte IR protocol
// Reference: https://hackaday.io/project/205909-hacking-mitsubishi-heavy-ac-ir-with-esp32

enum MitsubishiSRKMode : uint8_t {
  kSRKModeHeat = 0x3,
  kSRKModeFan = 0x4,
  kSRKModeDry = 0x5,
  kSRKModeCool = 0x6,
  kSRKModeAuto = 0x7,
};

enum MitsubishiSRKFan : uint8_t {
  kSRKFanLow = 0,
  kSRKFanMedium = 1,
  kSRKFanHigh = 2,
};

struct MitsubishiSRKState {
  bool power = false;
  MitsubishiSRKMode mode = kSRKModeCool;
  uint8_t temp_c = 26;
  MitsubishiSRKFan fan = kSRKFanMedium;
  uint8_t frame[8] = {0};
  bool valid = false;
};

struct MitsubishiSRKTiming {
  static constexpr uint16_t kHdrMark = 5950;
  static constexpr uint16_t kHdrSpace = 7475;
  static constexpr uint16_t kBitMark = 508;
  static constexpr uint16_t kOneSpace = 3454;
  static constexpr uint16_t kZeroSpace = 1496;
  static constexpr uint16_t kTrlSpace = 7422;
  static constexpr uint16_t kCarrierHz = 38000;
  static constexpr uint8_t kRepeatCount = 3;
  static constexpr uint16_t kRepeatGapMs = 50;
};

class MitsubishiSRK8 {
 public:
  // Decode one 8-byte frame from IRrecv raw buffer (microseconds internally).
  // volatile: IRremoteESP8266 3.x exposes rawbuf as atomic/volatile
  static bool decodeRawBuffer(volatile uint16_t const* rawbuf, uint16_t rawlen,
                              uint8_t frame_out[8],
                              uint16_t tick_us = 2);

  // Parse 8 bytes into human-readable AC state.
  static bool parseFrame(const uint8_t frame[8], MitsubishiSRKState* state);

  // Build 8-byte frame from desired state.
  static void buildFrame(const MitsubishiSRKState& state, uint8_t frame[8]);

  // Build raw timing array for IRsend.sendRaw() — returns length written.
  static size_t encodeRaw(const uint8_t frame[8], uint16_t* raw_out,
                          size_t raw_max);

  // Convenience: state → raw timings
  static size_t encodeState(const MitsubishiSRKState& state, uint16_t* raw_out,
                            size_t raw_max);

  static const char* modeToString(MitsubishiSRKMode mode);
  static const char* fanToString(MitsubishiSRKFan fan);

  static void printFrame(const uint8_t frame[8]);
  static void printState(const MitsubishiSRKState& state);

  // Debug: dump first N rawbuf entries (shown when decode fails).
  static void printRawPreview(volatile uint16_t const* rawbuf, uint16_t rawlen,
                              uint16_t tick_us, uint8_t count = 24);

 private:
  static bool validateFrame(const uint8_t frame[8]);
  static MitsubishiSRKFan fanFromB2(uint8_t b2);
  static void fanToB2B3(MitsubishiSRKFan fan, uint8_t* b2, uint8_t* b3);
};
