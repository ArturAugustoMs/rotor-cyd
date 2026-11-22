#pragma once

#include <cstdint>

namespace config {

constexpr char kFirmwareVersion[] = "1.1-rotor-web";
constexpr char kPortalApName[] = "Rotor-Setup";
constexpr char kPortalIp[] = "192.168.4.1";
constexpr char kPortalHostname[] = "rotor-cyd";

// Valores padrão dos quadros "Sobre" enquanto nada foi salvo pelo site.
// Podem ser alterados em Configurar -> "Indicativo (Sobre)" / "Nome (Sobre)".
constexpr char kOwnerCallsign[] = "PY0XXX";
constexpr char kOwnerName[] = "Seu Nome";

// Display (HSPI)
constexpr int kPinTftSclk = 14;
constexpr int kPinTftMosi = 13;
constexpr int kPinTftMiso = 12;
constexpr int kPinTftCs = 15;
constexpr int kPinTftDc = 2;
constexpr int kPinTftRst = -1;
constexpr int kPinTftBacklight = 21;
constexpr uint32_t kTftSpiWriteHz = 20000000;
constexpr uint32_t kTftSpiReadHz = 16000000;

// Touch XPT2046 (VSPI)
constexpr int kPinTouchSclk = 25;
constexpr int kPinTouchMosi = 32;
constexpr int kPinTouchMiso = 39;
constexpr int kPinTouchCs = 33;
constexpr int kPinTouchIrq = 36;

// Rotor relays and azimuth potentiometer
constexpr int kPinRelayCw = 22;
constexpr int kPinRelayCcw = 27;
constexpr int kPinPotRotor = 34;

constexpr int kScreenW = 320;
constexpr int kScreenH = 240;

}  // namespace config
