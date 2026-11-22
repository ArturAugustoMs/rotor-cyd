#include "hardware/motor.h"

#include <Arduino.h>

#include "config.h"

namespace motor {
namespace {

// Relés ativos em nível baixo: HIGH = desligado, LOW = ligado.
constexpr unsigned long kMaxContinuousRunMs = 60000UL;

int8_t s_direction = 0;
bool s_latchedOff = false;
unsigned long s_directionSince = 0;

void apply(int8_t direction) {
  const bool cw = direction > 0;
  const bool ccw = direction < 0;
  digitalWrite(config::kPinRelayCw, cw ? LOW : HIGH);
  digitalWrite(config::kPinRelayCcw, ccw ? LOW : HIGH);
}

}  // namespace

void init() {
  pinMode(config::kPinRelayCw, OUTPUT);
  pinMode(config::kPinRelayCcw, OUTPUT);
  s_direction = 0;
  s_latchedOff = false;
  s_directionSince = millis();
  allStop();
}

void set(int8_t direction) {
  if (direction < -1) direction = -1;
  if (direction > 1) direction = 1;

  if (direction == 0) {
    s_direction = 0;
    s_latchedOff = false;
    apply(0);
    return;
  }

  if (direction != s_direction) {
    s_direction = direction;
    s_latchedOff = false;
    s_directionSince = millis();
  }

  // Segurança: nunca deixa o motor girar sem limite se o comando "grudar".
  if (!s_latchedOff && millis() - s_directionSince > kMaxContinuousRunMs) {
    s_latchedOff = true;
    Serial.println("Motor: limite de 60s atingido - desligando");
  }

  apply(s_latchedOff ? 0 : s_direction);
}

void allStop() {
  s_direction = 0;
  s_latchedOff = false;
  apply(0);
}

}  // namespace motor
