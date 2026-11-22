#pragma once

#include <cstdint>

// QR do AP "Rotor-Setup" (WIFI:T:nopass;S:Rotor-Setup;;).
// Gerado por scripts/qr; 1 bit por pixel, MSB primeiro.
constexpr int kApQrWidth = 99;
constexpr int kApQrHeight = 99;
extern const uint8_t kApQrBits[];
