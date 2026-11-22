#pragma once

#include <LovyanGFX.hpp>

/** Embedded anti-aliased Noto Sans fonts: 0=XS 11px, 1=S 13px, 2=M 15px, 3=L 22px, 4=XL 32px. */
/** 5 = apenas dígitos 0-9 em 48px, ampliados da fonte XL com interpolação bicúbica. */
constexpr int kUiFontCount = 6;
constexpr int kDigitFontIndex = 5;

bool displayFontInit();
const lgfx::IFont* uiFont(int index);
/** Raw VLW data for a size (provided by font_blobs.cpp). */
const uint8_t* uiFontBlob(int index, size_t* len);
