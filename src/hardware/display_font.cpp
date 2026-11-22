#include "hardware/display_font.h"

#include <Arduino.h>

namespace {

lgfx::PointerWrapper s_data[kUiFontCount];
lgfx::VLWfont s_font[kUiFontCount];
bool s_ok = false;

const lgfx::IFont* fallback(int index) {
  switch (index) {
    case 0: return &lgfx::fonts::Font0;
    case 1:
    case 2: return &lgfx::fonts::DejaVu12;
    case 3:
    case 4:
    case 5: return &lgfx::fonts::DejaVu24;
    default: return &lgfx::fonts::DejaVu24;
  }
}

}  // namespace

bool displayFontInit() {
  if (s_ok) return true;
  bool ok = true;
  for (int i = 0; i < kUiFontCount; ++i) {
    size_t len = 0;
    const uint8_t* blob = uiFontBlob(i, &len);
    if (blob == nullptr || len < 24) { ok = false; break; }
    s_data[i].set(blob, len);
    if (!s_font[i].loadFont(&s_data[i])) { ok = false; break; }
  }
  s_ok = ok;
  if (!ok) Serial.println("Smooth font load failed - using bitmap fallback");
  return ok;
}

const lgfx::IFont* uiFont(int index) {
  if (index < 0) index = 0;
  if (index >= kUiFontCount) index = kUiFontCount - 1;
  return s_ok ? static_cast<const lgfx::IFont*>(&s_font[index]) : fallback(index);
}
