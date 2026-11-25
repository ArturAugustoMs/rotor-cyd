#include "services/settings.h"

#include <Preferences.h>

#include <cstdio>

#include "config.h"

namespace settings {

namespace {
Preferences s_prefs;
Settings s_current;
bool s_changed = false;

String clean(const String& value, size_t max_len, const char* fallback) {
  String out = value;
  out.trim();
  if (out.length() == 0) return String(fallback);
  if (out.length() > max_len) out = out.substring(0, max_len);
  return out;
}

const char* const kDefaultNames[] = {"Estados Unidos", "Europa"};
const uint16_t kDefaultAngles[] = {320, 40};

void loadPresets() {
  // 255 = chave ainda não gravada: primeiro boot usa os dois presets padrão.
  const uint8_t stored = s_prefs.getUChar("p_count", 255);
  if (stored == 255) {
    s_current.presetCount = 2;
    for (uint8_t i = 0; i < 2; ++i) {
      s_current.presetNames[i] = kDefaultNames[i];
      s_current.presetAngles[i] = kDefaultAngles[i];
    }
    return;
  }

  s_current.presetCount = stored < kMaxPresets ? stored : kMaxPresets;
  for (uint8_t i = 0; i < s_current.presetCount; ++i) {
    char key[10];
    snprintf(key, sizeof(key), "p%u_n", i);
    const char* fallback = i < 2 ? kDefaultNames[i] : "Preset";
    s_current.presetNames[i] =
        clean(s_prefs.getString(key, ""), kPresetNameMax, fallback);
    snprintf(key, sizeof(key), "p%u_a", i);
    const uint16_t fallbackAngle = i < 2 ? kDefaultAngles[i] : 0;
    s_current.presetAngles[i] = static_cast<uint16_t>(clampAngle(
        s_prefs.getUShort(key, fallbackAngle)));
  }
}

void savePresets() {
  s_prefs.putUChar("p_count", s_current.presetCount);
  for (uint8_t i = 0; i < s_current.presetCount; ++i) {
    char key[10];
    snprintf(key, sizeof(key), "p%u_n", i);
    s_prefs.putString(key, s_current.presetNames[i]);
    snprintf(key, sizeof(key), "p%u_a", i);
    s_prefs.putUShort(key, s_current.presetAngles[i]);
  }
}
}  // namespace

int clampAngle(int angle) {
  if (angle < kPresetAngleMin) return kPresetAngleMin;
  if (angle > kPresetAngleMax) return kPresetAngleMax;
  return angle;
}

uint8_t presetCount() { return s_current.presetCount; }

const char* presetName(uint8_t index) {
  return index < s_current.presetCount ? s_current.presetNames[index].c_str() : "";
}

uint16_t presetAngle(uint8_t index) {
  return index < s_current.presetCount ? s_current.presetAngles[index] : 0;
}

bool addPreset(const String& name, int angle) {
  if (s_current.presetCount >= kMaxPresets) return false;
  const String cleanName = clean(name, kPresetNameMax, "");
  if (!cleanName.length()) return false;
  const uint8_t index = s_current.presetCount++;
  s_current.presetNames[index] = cleanName;
  s_current.presetAngles[index] = static_cast<uint16_t>(clampAngle(angle));
  savePresets();
  s_changed = true;
  return true;
}

bool editPreset(uint8_t index, const String& name, int angle) {
  if (index >= s_current.presetCount) return false;
  const String cleanName = clean(name, kPresetNameMax, "");
  if (!cleanName.length()) return false;
  s_current.presetNames[index] = cleanName;
  s_current.presetAngles[index] = static_cast<uint16_t>(clampAngle(angle));
  savePresets();
  s_changed = true;
  return true;
}

bool removePreset(uint8_t index) {
  if (index >= s_current.presetCount) return false;
  for (uint8_t i = index; i + 1 < s_current.presetCount; ++i) {
    s_current.presetNames[i] = s_current.presetNames[i + 1];
    s_current.presetAngles[i] = s_current.presetAngles[i + 1];
  }
  --s_current.presetCount;
  savePresets();
  s_changed = true;
  return true;
}

void init() {
  s_prefs.begin("rotor", false);
  s_current.antenna = clean(s_prefs.getString("antenna", ""), 24, "ROTOR ANTENA");
  s_current.callsign = clean(s_prefs.getString("callsign", ""), 12, "PU5ASM");
  const uint8_t stored = s_prefs.getUChar("palette", 0);
  s_current.palette = stored < kPaletteCount ? stored : 0;

  const uint8_t lang = s_prefs.getUChar("language", kLanguagePtBr);
  s_current.language = lang < kLanguageCount ? lang : kLanguagePtBr;

  const uint8_t bri = s_prefs.getUChar("brightness", kBrightnessDefault);
  s_current.brightness = constrain(bri, kBrightnessMin, kBrightnessMax);

  loadPresets();
  s_changed = true;
}

Settings& get() { return s_current; }

void save() {
  s_prefs.putString("antenna", s_current.antenna);
  s_prefs.putString("callsign", s_current.callsign);
  s_prefs.putUChar("palette", s_current.palette);
  s_prefs.putUChar("language", s_current.language);
  s_prefs.putUChar("brightness", s_current.brightness);
  savePresets();
  s_changed = true;
}

bool consumeChanged() {
  const bool changed = s_changed;
  s_changed = false;
  return changed;
}

void markChanged() { s_changed = true; }

}  // namespace settings
