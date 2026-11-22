#include "services/settings.h"

#include <Preferences.h>

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
}  // namespace

void init() {
  s_prefs.begin("rotor", false);
  s_current.antenna = clean(s_prefs.getString("antenna", ""), 24, "ROTOR ANTENA");
  s_current.callsign = clean(s_prefs.getString("callsign", ""), 12, "PY2ROTOR");
  s_current.ownerCallsign =
      clean(s_prefs.getString("owner_callsign", ""), 12, config::kOwnerCallsign);
  s_current.ownerName =
      clean(s_prefs.getString("owner_name", ""), 24, config::kOwnerName);
  const uint8_t stored = s_prefs.getUChar("palette", 0);
  s_current.palette = stored < kPaletteCount ? stored : 0;

  const uint8_t lang = s_prefs.getUChar("language", kLanguagePtBr);
  s_current.language = lang < kLanguageCount ? lang : kLanguagePtBr;

  const uint8_t bri = s_prefs.getUChar("brightness", kBrightnessDefault);
  s_current.brightness = constrain(bri, kBrightnessMin, kBrightnessMax);

  s_changed = true;
}

Settings& get() { return s_current; }

void save() {
  s_prefs.putString("antenna", s_current.antenna);
  s_prefs.putString("callsign", s_current.callsign);
  s_prefs.putString("owner_callsign", s_current.ownerCallsign);
  s_prefs.putString("owner_name", s_current.ownerName);
  s_prefs.putUChar("palette", s_current.palette);
  s_prefs.putUChar("language", s_current.language);
  s_prefs.putUChar("brightness", s_current.brightness);
  s_changed = true;
}

bool consumeChanged() {
  const bool changed = s_changed;
  s_changed = false;
  return changed;
}

void markChanged() { s_changed = true; }

}  // namespace settings
