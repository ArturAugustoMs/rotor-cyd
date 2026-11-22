#pragma once

#include <cstdint>

namespace hmi {

/** Resultado da leitura de entrada na iteração atual do loop. */
struct Input {
  int8_t direction = 0;    // -1 = anti-horário, 0 = sem toque, +1 = horário
  bool rotorBlocked = false;  // true enquanto um menu de configuração está aberto
  bool resetNetwork = false;  // true uma vez, após confirmar "RESETAR REDE"
};

/** Inicializa display, fontes, buffer da bússola e calibra o touch. */
void init();

/** Processa toques e devolve a ação do usuário. */
Input process();

/** Atualiza setas de rotação (somente na tela principal). */
void setDirection(int8_t direction);

/** Filtra o azimute, atualiza o valor da web e redesenha a bússola. */
void updateDisplay(int azimuth);

/** Redesenha o cabeçalho se o nome/indicativo mudou pela web. */
void refreshHeaderIfChanged();

/** Mostra/oculta a tela do AP (QR Code) conforme o portal de configuração. */
void setApSetupMode(bool active);

/** Volta a desenhar a interface principal. */
void restoreBase();

}  // namespace hmi
