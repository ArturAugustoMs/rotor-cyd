#pragma once

#include <Arduino.h>

namespace settings {

/** Número de paletas disponíveis no CYD. */
constexpr uint8_t kPaletteCount = 5;

/** Idioma da interface: 0 = PT-BR (padrão), 1 = EN. */
constexpr uint8_t kLanguagePtBr = 0;
constexpr uint8_t kLanguageEn = 1;
constexpr uint8_t kLanguageCount = 2;

/** Brilho do display, em porcentagem. */
constexpr uint8_t kBrightnessMin = 10;
constexpr uint8_t kBrightnessMax = 100;
constexpr uint8_t kBrightnessDefault = 100;

struct Settings {
  String antenna;
  String callsign;
  /** Dados do quadro "Sobre" (configuráveis pelo site). */
  String ownerCallsign;
  String ownerName;
  uint8_t palette = 0;
  uint8_t language = kLanguagePtBr;
  uint8_t brightness = kBrightnessDefault;
};

void init();
Settings& get();
void save();
/** True once after a change; used by the UI to redraw the header. */
bool consumeChanged();
void markChanged();

}  // namespace settings
