#pragma once

#include <cstdint>

namespace rotor {

/** Fins de curso mecânicos do rotor (graus). */
constexpr int kAngleMin = 0;
constexpr int kAngleMax = 360;
/** Margem para considerar o alvo atingido sem ficar oscilando. */
constexpr int kTargetToleranceDeg = 2;
/** Aborta antes do corte de 60 s do motor.cpp. */
constexpr unsigned long kTargetTimeoutMs = 55000UL;
/** Sem variação de ângulo nesse tempo => travamento/fim de curso. */
constexpr unsigned long kTargetStallMs = 4000UL;

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

// ---------------------------------------------------------------------------
// Presets: alvo de azimute com segurança de fim de curso
// ---------------------------------------------------------------------------

/** Define um alvo (limitado a 0..360) e cancela qualquer comando manual. */
void setTarget(int angle);

/** Cancela o alvo atual e para o rotor. */
void clearTarget();

bool targetActive();
int targetAngle();

/**
 * Direção para alcançar o alvo, ou 0 quando chegou/abortou.
 * Nunca escolhe o caminho "curto" através de 0°/360°, pois existem batentes.
 */
int8_t targetCommand(int currentAzimuth);

}  // namespace rotor
