#pragma once

#include <cstdint>

namespace calibration {

/** Carrega a calibração salva na flash (se houver e for íntegra). */
void init();

bool isValid();
/** 8 parâmetros brutos para setTouchCalibrate(), ou nullptr se inválido. */
const uint16_t* params();

/** Grava os 8 parâmetros com checksum. */
void save(const uint16_t values[8]);

/** Apaga a calibração (usado no reset de rede). */
void clear();

}  // namespace calibration
