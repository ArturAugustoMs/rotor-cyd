#pragma once

#include <cstdint>

namespace rotor {

/** -1 = anti-horário, 0 = parado, +1 = horário. */
void setWebCommand(int8_t direction);

/** Comando da web com watchdog: volta a 0 se o cliente parar de enviar. */
int8_t webCommand();

/** Último comando recebido, sem efeito de watchdog (usado pelo /api/state). */
int8_t commandedDirection();

/** Azimute lido do potenciômetro (0..360). */
void updateAzimuth(int azimuth);
int currentAzimuth();

/** Azimute suavizado que está realmente desenhado na tela (0..360). */
void updateDisplayedAzimuth(int azimuth);
int displayedAzimuth();

}  // namespace rotor
