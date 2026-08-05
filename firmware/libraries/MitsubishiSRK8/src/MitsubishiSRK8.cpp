#include "MitsubishiSRK8.h"

#include <Arduino.h>
#include <string.h>

namespace {

constexpr uint32_t kSpaceThresholdUs = 2400;  // between ZERO (1496) and ONE (3454)

uint32_t rawToUs(volatile uint16_t const* rawbuf, int idx, uint16_t tick_us) {
  return static_cast<uint32_t>(rawbuf[idx]) * tick_us;
}

bool matchHeader(volatile uint16_t const* rawbuf, uint16_t rawlen, int mark_idx,
                 uint16_t tick_us) {
  if (mark_idx < 0 || mark_idx + 1 >= static_cast<int>(rawlen)) {
    return false;
  }
  const uint32_t mark_us = rawToUs(rawbuf, mark_idx, tick_us);
  const uint32_t space_us = rawToUs(rawbuf, mark_idx + 1, tick_us);
  return (mark_us >= 4500 && mark_us <= 7500) &&
         (space_us >= 6000 && space_us <= 9000);
}

bool extractFrame(volatile uint16_t const* rawbuf, uint16_t rawlen, int hdr_idx,
                  uint16_t tick_us, uint8_t frame_out[8]) {
  const int bit_start = hdr_idx + 2;  // after header mark + space

  memset(frame_out, 0, 8);
  for (int bit_idx = 0; bit_idx < 64; bit_idx++) {
    const int space_idx = bit_start + bit_idx * 2 + 1;
    if (space_idx >= static_cast<int>(rawlen)) {
      return false;
    }
    const uint32_t space_us = rawToUs(rawbuf, space_idx, tick_us);
    if (space_us >= kSpaceThresholdUs) {
      const int byte_idx = bit_idx / 8;
      const int bit_pos = bit_idx % 8;  // LSB first
      frame_out[byte_idx] |= static_cast<uint8_t>(1U << bit_pos);
    }
  }
  return true;
}

}  // namespace

bool MitsubishiSRK8::decodeRawBuffer(volatile uint16_t const* rawbuf,
                                     uint16_t rawlen, uint8_t frame_out[8],
                                     uint16_t tick_us) {
  if (rawbuf == nullptr || frame_out == nullptr || rawlen < 50) {
    return false;
  }

  memset(frame_out, 0, 8);

  // Try microsecond-native (1), caller tick, and classic 2 µs ticks.
  const uint16_t tick_candidates[] = {1, tick_us, 2};

  for (uint8_t t = 0; t < 3; t++) {
    const uint16_t tick = tick_candidates[t];
    if (tick == 0) {
      continue;
    }

    // Scan entire buffer — handles leading gap and multi-frame captures.
    for (int hdr_idx = 0; hdr_idx + 1 < static_cast<int>(rawlen); hdr_idx++) {
      if (!matchHeader(rawbuf, rawlen, hdr_idx, tick)) {
        continue;
      }

      uint8_t frame[8];
      if (!extractFrame(rawbuf, rawlen, hdr_idx, tick, frame)) {
        continue;
      }
      if (validateFrame(frame)) {
        memcpy(frame_out, frame, 8);
        return true;
      }
    }
  }

  // Fallback: return best-effort frame from first detected header (for debug).
  for (uint16_t tick : tick_candidates) {
    if (tick == 0) {
      continue;
    }
    for (int hdr_idx = 0; hdr_idx + 1 < static_cast<int>(rawlen); hdr_idx++) {
      if (matchHeader(rawbuf, rawlen, hdr_idx, tick)) {
        extractFrame(rawbuf, rawlen, hdr_idx, tick, frame_out);
        return false;
      }
    }
  }

  return false;
}

bool MitsubishiSRK8::validateFrame(const uint8_t frame[8]) {
  const bool header_ok = (frame[0] == 0xFF && frame[1] == 0x00);
  const bool tail_ok = (frame[6] == 0x2A && frame[7] == 0xD5);
  const bool fan_chk = (frame[3] == static_cast<uint8_t>(0xFF - frame[2]));
  const bool mode_chk = (frame[5] == static_cast<uint8_t>(0xFF - frame[4]));
  return header_ok && tail_ok && fan_chk && mode_chk;
}

void MitsubishiSRK8::printRawPreview(volatile uint16_t const* rawbuf,
                                     uint16_t rawlen, uint16_t tick_us,
                                     uint8_t count) {
  if (rawbuf == nullptr) {
    return;
  }

  Serial.printf("Raw preview (tick=%u, len=%u):\n", tick_us, rawlen);
  if (count > rawlen) {
    count = static_cast<uint8_t>(rawlen);
  }
  for (uint8_t i = 0; i < count; i++) {
    Serial.printf("  [%u] raw=%u  us~%lu\n", i, rawbuf[i],
                    rawToUs(rawbuf, i, tick_us));
  }
}

bool MitsubishiSRK8::parseFrame(const uint8_t frame[8], MitsubishiSRKState* state) {
  if (state == nullptr || !validateFrame(frame)) {
    return false;
  }

  memcpy(state->frame, frame, 8);

  state->fan = fanFromB2(frame[2]);

  const uint8_t temp_code = frame[4] >> 4;
  uint8_t mode_code = frame[4] & 0x0F;
  state->power = !(mode_code & 0x08);
  mode_code &= 0x07;

  switch (mode_code) {
    case kSRKModeHeat:
      state->mode = kSRKModeHeat;
      break;
    case kSRKModeFan:
      state->mode = kSRKModeFan;
      break;
    case kSRKModeDry:
      state->mode = kSRKModeDry;
      break;
    case kSRKModeCool:
      state->mode = kSRKModeCool;
      break;
    case kSRKModeAuto:
    default:
      state->mode = kSRKModeAuto;
      break;
  }

  state->temp_c = static_cast<uint8_t>(32 - temp_code);
  state->valid = true;
  return true;
}

MitsubishiSRKFan MitsubishiSRK8::fanFromB2(uint8_t b2) {
  if (b2 == 0xFF) {
    return kSRKFanHigh;
  }
  if (b2 == 0xBF) {
    return kSRKFanMedium;
  }
  return kSRKFanLow;
}

void MitsubishiSRK8::fanToB2B3(MitsubishiSRKFan fan, uint8_t* b2, uint8_t* b3) {
  switch (fan) {
    case kSRKFanHigh:
      *b2 = 0xFF;
      *b3 = 0x00;
      break;
    case kSRKFanMedium:
      *b2 = 0xBF;
      *b3 = 0x40;
      break;
    case kSRKFanLow:
    default:
      *b2 = 0x9F;
      *b3 = 0x60;
      break;
  }
}

void MitsubishiSRK8::buildFrame(const MitsubishiSRKState& state, uint8_t frame[8]) {
  uint8_t b2 = 0xBF;
  uint8_t b3 = 0x40;
  fanToB2B3(state.fan, &b2, &b3);

  uint8_t mode_code = state.mode;
  if (!state.power) {
    mode_code |= 0x08;
  }

  const uint8_t temp_code = static_cast<uint8_t>((32 - state.temp_c) & 0x0F);
  const uint8_t b4 = static_cast<uint8_t>((temp_code << 4) | (mode_code & 0x0F));
  const uint8_t b5 = static_cast<uint8_t>(0xFF - b4);

  frame[0] = 0xFF;
  frame[1] = 0x00;
  frame[2] = b2;
  frame[3] = b3;
  frame[4] = b4;
  frame[5] = b5;
  frame[6] = 0x2A;
  frame[7] = 0xD5;
}

size_t MitsubishiSRK8::encodeRaw(const uint8_t frame[8], uint16_t* raw_out,
                                 size_t raw_max) {
  if (raw_out == nullptr || raw_max < 132) {
    return 0;
  }

  size_t idx = 0;
  raw_out[idx++] = MitsubishiSRKTiming::kHdrMark;
  raw_out[idx++] = MitsubishiSRKTiming::kHdrSpace;

  for (int b = 0; b < 8; b++) {
    for (int bit = 0; bit < 8; bit++) {
      raw_out[idx++] = MitsubishiSRKTiming::kBitMark;
      raw_out[idx++] = (frame[b] >> bit) & 1 ? MitsubishiSRKTiming::kOneSpace
                                             : MitsubishiSRKTiming::kZeroSpace;
    }
  }

  raw_out[idx++] = MitsubishiSRKTiming::kBitMark;
  raw_out[idx++] = MitsubishiSRKTiming::kTrlSpace;
  return idx;
}

size_t MitsubishiSRK8::encodeState(const MitsubishiSRKState& state,
                                   uint16_t* raw_out, size_t raw_max) {
  uint8_t frame[8];
  buildFrame(state, frame);
  return encodeRaw(frame, raw_out, raw_max);
}

const char* MitsubishiSRK8::modeToString(MitsubishiSRKMode mode) {
  switch (mode) {
    case kSRKModeHeat:
      return "heat";
    case kSRKModeFan:
      return "fan";
    case kSRKModeDry:
      return "dry";
    case kSRKModeCool:
      return "cool";
    case kSRKModeAuto:
    default:
      return "auto";
  }
}

const char* MitsubishiSRK8::fanToString(MitsubishiSRKFan fan) {
  switch (fan) {
    case kSRKFanHigh:
      return "high";
    case kSRKFanMedium:
      return "medium";
    case kSRKFanLow:
    default:
      return "low";
  }
}

void MitsubishiSRK8::printFrame(const uint8_t frame[8]) {
  Serial.print("Frame: ");
  for (int i = 0; i < 8; i++) {
    Serial.printf("%02X ", frame[i]);
  }
  Serial.println();
}

void MitsubishiSRK8::printState(const MitsubishiSRKState& state) {
  Serial.printf("Power : %s\n", state.power ? "ON" : "OFF");
  Serial.printf("Mode  : %s\n", modeToString(state.mode));
  Serial.printf("Temp  : %u C\n", state.temp_c);
  Serial.printf("Fan   : %s\n", fanToString(state.fan));
  printFrame(state.frame);
}
