#pragma once

#include <cstdint>

namespace motor {

/** Configura os pinos e garante que os relés comecem desligados. */
void init();

/** -1 = anti-horário, 0 = parado, +1 = horário. */
void set(int8_t direction);

/** Parada imediata (usada antes de reiniciar ou abrir a configuração). */
void allStop();

}  // namespace motor
