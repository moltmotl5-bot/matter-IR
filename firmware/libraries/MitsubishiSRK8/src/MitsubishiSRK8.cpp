#include "MitsubishiSRK8.h"

#include <Arduino.h>
#include <string.h>

bool MitsubishiSRK8::decodeRawBuffer(const uint16_t* rawbuf, uint16_t rawlen,
                                     uint8_t frame_out[8], uint16_t tick_us) {
  if (rawbuf == nullptr || frame_out == nullptr || rawlen < 131) {
    return false;
  }

  // Space-length encoding: read odd-index spaces after header (indices 4+).
  // Threshold ~2500 µs between ZERO_SPACE (1496) and ONE_SPACE (3454).
  const uint16_t one_threshold = 2500 / tick_us;

  memset(frame_out, 0, 8);
  for (int b = 0; b < 8; b++) {
    for (int bit = 0; bit < 8; bit++) {
      const int space_idx = 4 + (b * 8 + bit) * 2;
      if (space_idx >= rawlen) {
        return false;
      }
      if (rawbuf[space_idx] > one_threshold) {
        frame_out[b] |= static_cast<uint8_t>(1U << bit);
      }
    }
  }

  return validateFrame(frame_out);
}

bool MitsubishiSRK8::validateFrame(const uint8_t frame[8]) {
  const bool header_ok = (frame[0] == 0xFF && frame[1] == 0x00);
  const bool tail_ok = (frame[6] == 0x2A && frame[7] == 0xD5);
  const bool fan_chk = (frame[3] == static_cast<uint8_t>(0xFF - frame[2]));
  const bool mode_chk = (frame[5] == static_cast<uint8_t>(0xFF - frame[4]));
  return header_ok && tail_ok && fan_chk && mode_chk;
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
