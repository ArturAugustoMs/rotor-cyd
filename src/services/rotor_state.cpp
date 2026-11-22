#include "services/rotor_state.h"

#include <Arduino.h>

namespace rotor {
namespace {

volatile int8_t s_webDir = 0;
volatile unsigned long s_webCmdMs = 0;
volatile int s_azimuth = 0;
volatile int s_displayAzimuth = 0;

constexpr unsigned long kCommandTimeoutMs = 1200UL;

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

}  // namespace rotor
