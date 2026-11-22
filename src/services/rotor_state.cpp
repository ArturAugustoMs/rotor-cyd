#include "services/rotor_state.h"

#include <Arduino.h>

namespace rotor {
namespace {

volatile int8_t s_webDir = 0;
volatile unsigned long s_webCmdMs = 0;
volatile int s_azimuth = 0;
volatile int s_displayAzimuth = 0;

constexpr unsigned long kCommandTimeoutMs = 1200UL;

// Alvo dos presets. -1 = sem alvo ativo.
volatile int s_target = -1;
volatile bool s_targetActive = false;
volatile unsigned long s_targetSince = 0;
volatile unsigned long s_progressMs = 0;
volatile int s_progressAngle = 0;
volatile int8_t s_targetDir = 0;

int distance(int a, int b) { return a > b ? a - b : b - a; }

}  // namespace

void setWebCommand(int8_t direction) {
  if (direction < -1) direction = -1;
  if (direction > 1) direction = 1;
  s_webDir = direction;
  s_webCmdMs = millis();
}

int8_t webCommand() {
  if (s_webDir != 0 && millis() - s_webCmdMs > kCommandTimeoutMs) {
    s_webDir = 0;
  }
  return s_webDir;
}

int8_t commandedDirection() { return s_webDir; }

void updateAzimuth(int azimuth) { s_azimuth = azimuth; }
int currentAzimuth() { return s_azimuth; }

void updateDisplayedAzimuth(int azimuth) { s_displayAzimuth = azimuth; }
int displayedAzimuth() { return s_displayAzimuth; }

void setTarget(int angle) {
  if (angle < kAngleMin) angle = kAngleMin;
  if (angle > kAngleMax) angle = kAngleMax;

  // Um preset substitui qualquer comando de seta que ainda estivesse ativo.
  s_webDir = 0;
  s_webCmdMs = 0;

  s_target = angle;
  s_targetActive = true;
  s_targetSince = millis();
  s_progressMs = s_targetSince;
  s_progressAngle = s_azimuth;
  s_targetDir = 0;
  Serial.printf("Preset: alvo %d graus (atual %d)\n", angle, s_azimuth);
}

void clearTarget() {
  s_targetActive = false;
  s_target = -1;
  s_targetDir = 0;
}

bool targetActive() { return s_targetActive; }
int targetAngle() { return s_target; }

int8_t targetCommand(int currentAzimuth) {
  if (!s_targetActive) return 0;

  if (currentAzimuth < kAngleMin) currentAzimuth = kAngleMin;
  if (currentAzimuth > kAngleMax) currentAzimuth = kAngleMax;

  const unsigned long now = millis();
  if (now - s_targetSince > kTargetTimeoutMs) {
    Serial.println("Preset abortado: tempo limite (55 s)");
    clearTarget();
    return 0;
  }

  // Nos extremos usa uma margem maior: nunca "empurra" o rotor contra o batente
  // por causa de alguns graus de erro do potenciômetro.
  const int tolerance =
      (s_target == kAngleMin || s_target == kAngleMax) ? 5 : kTargetToleranceDeg;
  if (distance(s_target, currentAzimuth) <= tolerance) {
    Serial.printf("Preset: alvo %d atingido\n", s_target);
    clearTarget();
    return 0;
  }

  // Com batentes nas extremidades, nunca se pode "dar a volta" por 0/360.
  const int8_t dir = s_target > currentAzimuth ? 1 : -1;
  if ((dir < 0 && currentAzimuth <= kAngleMin) ||
      (dir > 0 && currentAzimuth >= kAngleMax)) {
    Serial.println("Preset abortado: fim de curso");
    clearTarget();
    return 0;
  }

  if (dir != s_targetDir) {
    s_targetDir = dir;
    s_progressMs = now;
    s_progressAngle = currentAzimuth;
  } else if (now - s_progressMs > kTargetStallMs) {
    if (distance(currentAzimuth, s_progressAngle) < 2) {
      Serial.println("Preset abortado: rotor sem avanco");
      clearTarget();
      return 0;
    }
    s_progressMs = now;
    s_progressAngle = currentAzimuth;
  }

  return dir;
}

}  // namespace rotor
