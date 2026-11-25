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

/** Presets de azimute (fins de curso físicos em 0° e 360°). */
constexpr uint8_t kMaxPresets = 10;
constexpr size_t kPresetNameMax = 20;
constexpr int kPresetAngleMin = 0;
constexpr int kPresetAngleMax = 360;

struct Settings {
  String antenna;
  String callsign;
  uint8_t palette = 0;
  uint8_t language = kLanguagePtBr;
  uint8_t brightness = kBrightnessDefault;

  uint8_t presetCount = 0;
  String presetNames[kMaxPresets];
  uint16_t presetAngles[kMaxPresets];
};

/** Limita um ângulo ao intervalo mecânico do rotor (sem dar a volta). */
int clampAngle(int angle);

uint8_t presetCount();
const char* presetName(uint8_t index);
uint16_t presetAngle(uint8_t index);
/** Inclui um preset. Retorna false se a lista estiver cheia ou inválida. */
bool addPreset(const String& name, int angle);
bool editPreset(uint8_t index, const String& name, int angle);
bool removePreset(uint8_t index);

void init();
Settings& get();
void save();
/** True once after a change; used by the UI to redraw the header. */
bool consumeChanged();
void markChanged();

}  // namespace settings
