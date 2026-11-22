#include "services/calibration.h"

#include <Arduino.h>
#include <Preferences.h>

namespace calibration {
namespace {

Preferences prefs;
uint16_t s_params[8] = {0};
bool s_valid = false;

constexpr char kKeyData[] = "data";
constexpr char kKeyValid[] = "ok";
constexpr size_t kDataSize = 18;  // 8 uint16_t + 1 checksum uint16_t

uint16_t checksum(const uint16_t values[8]) {
  uint16_t sum = 0;
  for (int i = 0; i < 8; ++i) {
    sum = static_cast<uint16_t>(sum + values[i] + static_cast<uint16_t>(i + 1));
  }
  return sum;
}

}  // namespace

void init() {
  prefs.begin("rotor_cal", false);

  if (prefs.getBool(kKeyValid, false) && prefs.getBytesLength(kKeyData) == kDataSize) {
    uint8_t buffer[kDataSize] = {0};
    prefs.getBytes(kKeyData, buffer, kDataSize);

    uint16_t values[8] = {0};
    for (int i = 0; i < 8; ++i) {
      values[i] = static_cast<uint16_t>(buffer[i * 2] | (buffer[i * 2 + 1] << 8));
    }
    const uint16_t stored = static_cast<uint16_t>(buffer[16] | (buffer[17] << 8));

    if (stored == checksum(values)) {
      for (int i = 0; i < 8; ++i) s_params[i] = values[i];
      s_valid = true;
      Serial.println("Touch: calibracao carregada da flash");
    }
  }

  if (!s_valid) Serial.println("Touch: sem calibracao salva");
}

bool isValid() { return s_valid; }

const uint16_t* params() { return s_valid ? s_params : nullptr; }

void save(const uint16_t values[8]) {
  uint8_t buffer[kDataSize] = {0};
  for (int i = 0; i < 8; ++i) {
    buffer[i * 2] = static_cast<uint8_t>(values[i] & 0xFF);
    buffer[i * 2 + 1] = static_cast<uint8_t>(values[i] >> 8);
  }
  const uint16_t cs = checksum(values);
  buffer[16] = static_cast<uint8_t>(cs & 0xFF);
  buffer[17] = static_cast<uint8_t>(cs >> 8);

  prefs.putBytes(kKeyData, buffer, kDataSize);
  prefs.putBool(kKeyValid, true);

  for (int i = 0; i < 8; ++i) s_params[i] = values[i];
  s_valid = true;
  Serial.println("Touch: calibracao gravada na flash");
}

void clear() {
  prefs.remove(kKeyData);
  prefs.remove(kKeyValid);
  s_valid = false;
  Serial.println("Touch: calibracao apagada");
}

}  // namespace calibration
