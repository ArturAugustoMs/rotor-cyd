#include "ui/hmi.h"

#include <Arduino.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "config.h"
#include "hardware/ap_qr.h"
#include "hardware/display_font.h"
#include "hardware/lgfx_rotor.hpp"
#include "services/calibration.h"
#include "services/rotor_state.h"
#include "services/settings.h"

// Comando "Gamma Set" (ajuste de gamma exigido pela configuração original).
#define ILI9341_GAMMASET 0x26

namespace hmi {
namespace {

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

LGFX tft;
LGFX_Sprite compassSprite(&tft);
bool compassSpriteOk = false;

const int kScreenW = 320;

const int CX = 160;
const int CY = 109;
const int ROSA_R = 64;
const int COMPASS_SC = ROSA_R + 14;
const int COMPASS_BOX = 2 * COMPASS_SC;

const int BTN_Y = 188;
const int BTN_W = 104;
const int BTN_H = 42;
const int BTN_CCW_X = 34;
const int BTN_CW_X = 182;

const int CFG_X = 292;
const int CFG_Y = 4;
const int CFG_W = 24;
const int CFG_H = 22;

constexpr float kDeg2Rad = 0.01745329251994329577f;

static constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

/** Mistura duas cores RGB565 (t de 0 a 255). */
static uint16_t blend(uint16_t a, uint16_t b, uint8_t t) {
  const int ar = (a >> 11) & 0x1F, ag = (a >> 5) & 0x3F, ab = a & 0x1F;
  const int br = (b >> 11) & 0x1F, bg = (b >> 5) & 0x3F, bb = b & 0x1F;
  return static_cast<uint16_t>(
      ((ar + (((br - ar) * t) >> 8)) << 11) |
      ((ag + (((bg - ag) * t) >> 8)) << 5) |
       (ab + (((bb - ab) * t) >> 8)));
}

/** Cores trocáveis pelo portal web. Verde/vermelho dos relés ficam fixos. */
struct Palette {
  uint16_t bg;        // fundo
  uint16_t panel;     // cartões e bússola
  uint16_t accent;    // destaques e ponteiros de texto
  uint16_t secondary; // anel da bússola
  uint16_t text;      // texto principal
  uint16_t muted;     // texto secundário
};

static const Palette kPalettes[settings::kPaletteCount] = {
    /* 0 Gelo/Teal  */ {rgb(4, 10, 28), rgb(8, 16, 38), rgb(0, 200, 255),
                        rgb(45, 212, 191), rgb(235, 240, 250), rgb(120, 140, 170)},
    /* 1 Floresta   */ {rgb(6, 20, 16), rgb(10, 32, 26), rgb(60, 220, 110),
                        rgb(45, 212, 191), rgb(230, 245, 235), rgb(120, 160, 140)},
    /* 2 Solar      */ {rgb(28, 14, 4), rgb(42, 24, 8), rgb(255, 176, 32),
                        rgb(255, 120, 60), rgb(250, 240, 225), rgb(180, 150, 115)},
    /* 3 Magenta    */ {rgb(22, 6, 28), rgb(38, 10, 46), rgb(255, 80, 200),
                        rgb(160, 110, 255), rgb(245, 235, 250), rgb(165, 130, 180)},
    /* 4 Grafite    */ {rgb(12, 14, 18), rgb(24, 28, 34), rgb(200, 215, 235),
                        rgb(120, 180, 220), rgb(240, 244, 250), rgb(140, 150, 165)},
};

// Referências estáveis: mudar `colors` atualiza todas as cores usadas no desenho.
Palette colors = kPalettes[0];
const uint16_t& C_BG = colors.bg;
const uint16_t& C_PANEL = colors.panel;
const uint16_t& C_CYAN = colors.accent;
const uint16_t& C_TEAL = colors.secondary;
const uint16_t& C_TEXT = colors.text;
const uint16_t& C_MUTED = colors.muted;

constexpr uint16_t C_GREEN = rgb(60, 220, 110);
constexpr uint16_t C_RED = rgb(255, 70, 70);

void applyPalette() {
  const uint8_t index = settings::get().palette;
  colors = kPalettes[index < settings::kPaletteCount ? index : 0];
}

// ---------------------------------------------------------------------------
// Textos da interface (PT-BR / EN)
// ---------------------------------------------------------------------------

enum Txt : uint8_t {
  T_CONFIG, T_CALIBRATE, T_RESET_WIFI, T_LANGUAGE, T_BRIGHTNESS, T_ABOUT,
  T_BACK, T_CALLSIGN, T_NAME, T_DATE, T_YEAR, T_VERSION,
  T_RESET_Q, T_ERASE_RESTART, T_CANCEL, T_CONFIRM, T_RESTARTING,
  T_CAL_TOUCH, T_CAL_POINT, T_CAL_HOLD, T_DEGREES,
  T_WIFI_SETUP, T_CONNECT_AP, T_NEXT, T_TIP, T_TIP_L1, T_TIP_L2, T_OK,
  T_COUNT
};

static const char* const kText[settings::kLanguageCount][T_COUNT] = {
    {  // PT-BR (padrão)
        "CONFIGURACOES", "CALIBRAR", "RESETAR REDE", "IDIOMA", "BRILHO",
        "SOBRE", "VOLTAR", "Indicativo", "Nome", "Data", "Ano", "Versao",
        "RESETAR REDE?", "Apagar Wi-Fi e reiniciar", "CANCELAR", "CONFIRMAR",
        "REINICIANDO...", "CALIBRAR TOUCH", "PONTO %d/4 - SENTIDO HORARIO",
        "Toque e segure cada alvo", "GRAUS",
        "CONFIGURAR WIFI", "Conecte-se ao AP:", "AVANCAR", "DICA",
        "Configure seu WiFi", "em 'Rede' no site", "ENTENDI"},
    {  // EN
        "SETTINGS", "CALIBRATE", "RESET WIFI", "LANGUAGE", "BRIGHTNESS",
        "ABOUT", "BACK", "Callsign", "Name", "Date", "Year", "Version",
        "RESET WIFI?", "Erase Wi-Fi and restart", "CANCEL", "CONFIRM",
        "RESTARTING...", "CALIBRATE TOUCH", "POINT %d/4 - CLOCKWISE",
        "Touch and hold each target", "DEGREES",
        "WIFI SETUP", "Connect to AP:", "NEXT", "TIP",
        "Configure your WiFi", "in 'Rede' on the site", "GOT IT"},
};

static const char* tr(Txt id) {
  const uint8_t lang = settings::get().language;
  return kText[lang < settings::kLanguageCount ? lang : settings::kLanguagePtBr][id];
}

static inline bool inside(int x, int y, int bx, int by, int bw, int bh) {
  return x >= bx && x <= bx + bw && y >= by && y <= by + bh;
}

enum class Screen : uint8_t { MAIN, CONFIG, ABOUT, BRIGHTNESS, AP_SETUP, CONFIRM_RESET };

Screen screen = Screen::MAIN;
int8_t lastDrawnDirection = 0;
bool previousTouch = false;
bool apSetupMode = false;
bool apTipVisible = false;

// ---------------------------------------------------------------------------
// Desenho
// ---------------------------------------------------------------------------

void drawHeader() {
  tft.fillRect(0, 0, kScreenW, 30, C_PANEL);
  tft.fillRect(0, 28, kScreenW, 2, C_CYAN);

  // Indicativo: mesma fonte do apelido, alinhado à esquerda da engrenagem,
  // sem caixa nem borda. O apelido é cortado para não sobrepor o indicativo.
  const String call = settings::get().callsign;
  const int callRight = CFG_X - 6;
  const int callWidth = tft.textWidth(call, uiFont(3));
  int nameLimit = callRight - callWidth - 12;
  if (nameLimit < 24) nameLimit = 24;

  String name = settings::get().antenna;
  while (name.length() > 1 && tft.textWidth(name, uiFont(3)) > nameLimit) {
    name.remove(name.length() - 1);
  }

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(C_TEXT, C_PANEL);
  tft.drawString(name.c_str(), 8, 3, uiFont(3));

  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(C_CYAN, C_PANEL);
  tft.drawString(call.c_str(), callRight, 3, uiFont(3));

  // Botão de menu: quadrado arredondado com três riscos arredondados.
  const int gx = CFG_X + CFG_W / 2;
  const int gy = CFG_Y + CFG_H / 2;
  tft.fillRoundRect(CFG_X, CFG_Y, CFG_W, CFG_H, 6, blend(C_PANEL, C_TEXT, 26));
  tft.drawRoundRect(CFG_X, CFG_Y, CFG_W, CFG_H, 6, blend(C_PANEL, C_MUTED, 70));
  for (int i = -1; i <= 1; ++i) {
    tft.fillRoundRect(gx - 6, gy + i * 5 - 1, 12, 3, 1, C_CYAN);
  }
}

static void fillQuad(lgfx::LovyanGFX& g,
                     int x0, int y0, int x1, int y1,
                     int x2, int y2, int x3, int y3, uint16_t color) {
  g.fillTriangle(x0, y0, x1, y1, x2, y2, color);
  g.fillTriangle(x0, y0, x2, y2, x3, y3, color);
}

void drawCompassInto(lgfx::LovyanGFX& g, int x0, int y0, int angle) {
  const int cx = x0 + COMPASS_SC;
  const int cy = y0 + COMPASS_SC;

  g.fillRect(x0, y0, COMPASS_BOX, COMPASS_BOX, C_BG);

  // Halo externo suave e mostrador com anel de brilho na borda.
  g.drawCircle(cx, cy, ROSA_R + 3, blend(C_BG, C_TEAL, 10));
  g.drawCircle(cx, cy, ROSA_R + 2, blend(C_BG, C_TEAL, 16));
  g.drawCircle(cx, cy, ROSA_R + 1, blend(C_BG, C_TEAL, 26));
  g.fillCircle(cx, cy, ROSA_R, C_PANEL);
  for (int i = 0; i < 4; ++i) {
    g.drawCircle(cx, cy, ROSA_R - i, blend(C_PANEL, C_TEAL, 64 - i * 13));
  }
  g.drawCircle(cx, cy, ROSA_R - 6, blend(C_PANEL, C_MUTED, 70));

  // Marcadores: cards em destaque, intermediários em teal, demais discretos.
  for (int a = 0; a < 360; a += 15) {
    const float rad = (a - 90) * kDeg2Rad;
    const bool cardinal = (a % 90 == 0);
    const bool major = (a % 45 == 0);
    const int ra = cardinal ? ROSA_R - 15 : (major ? ROSA_R - 11 : ROSA_R - 7);
    const int rb = ROSA_R - 4;
    const uint16_t color = cardinal ? C_TEXT : (major ? C_TEAL : C_MUTED);
    const int x1 = cx + cosf(rad) * ra, y1 = cy + sinf(rad) * ra;
    const int x2 = cx + cosf(rad) * rb, y2 = cy + sinf(rad) * rb;
    g.drawLine(x1, y1, x2, y2, color);
    if (!cardinal && !major) {
      g.fillCircle(cx + cosf(rad) * (rb + 2), cy + sinf(rad) * (rb + 2), 1, color);
    }
  }

  g.setTextColor(C_CYAN, C_BG);
  g.setTextDatum(MC_DATUM);
  g.drawString("N", cx, cy - ROSA_R - 6, uiFont(2));
  g.drawString("S", cx, cy + ROSA_R + 6, uiFont(2));
  g.drawString("E", cx + ROSA_R + 6, cy, uiFont(2));
  g.drawString("W", cx - ROSA_R - 6, cy, uiFont(2));

  char buffer[8];
  snprintf(buffer, sizeof(buffer), "%03d", angle);

  // Fonte nativa de 48px (dígitos ampliados da original com bicúbico):
  // antes a fonte de 32px era escalada em tempo real pelo LovyanGFX, que faz
  // replicação de pixels e deixava o texto com aspecto de blocos.
  g.setTextSize(1, 1);
  g.setTextColor(C_GREEN, C_PANEL);
  g.drawString(buffer, cx, cy, uiFont(kDigitFontIndex));

  g.setTextColor(C_MUTED, C_PANEL);
  g.drawString(tr(T_DEGREES), cx, cy + 36, uiFont(0));

  // Apenas a seta vermelha, igual à do site (sem contraponto nem cubo).
  const float rad = (angle - 90) * kDeg2Rad;
  const float dx = cosf(rad);
  const float dy = sinf(rad);
  const int tx = cx + dx * (ROSA_R - 2);
  const int ty = cy + dy * (ROSA_R - 2);
  const int bx = cx + dx * (ROSA_R - 13);
  const int by = cy + dy * (ROSA_R - 13);
  const int px = -dy * 5;
  const int py = dx * 5;
  g.fillTriangle(tx, ty, bx + px, by + py, bx - px, by - py, C_RED);
}

void drawCompass(int angle) {
  if (compassSpriteOk) {
    drawCompassInto(compassSprite, 0, 0, angle);
    compassSprite.pushSprite(&tft, CX - COMPASS_SC, CY - COMPASS_SC);
  } else {
    drawCompassInto(tft, CX - COMPASS_SC, CY - COMPASS_SC, angle);
  }
}

void drawArrowButton(int x, bool left, bool pressed) {
  const uint16_t color = left ? C_RED : C_GREEN;
  const int r = 12;
  tft.fillRoundRect(x, BTN_Y, BTN_W, BTN_H, r,
                    pressed ? color : blend(C_PANEL, C_TEXT, 15));
  tft.drawRoundRect(x, BTN_Y, BTN_W, BTN_H, r, color);
  // Realce superior e sombra inferior discretos.
  tft.drawLine(x + r, BTN_Y + 1, x + BTN_W - r, BTN_Y + 1,
               blend(C_PANEL, C_TEXT, pressed ? 70 : 42));
  tft.drawLine(x + r, BTN_Y + BTN_H - 2, x + BTN_W - r, BTN_Y + BTN_H - 2,
               blend(C_BG, color, 40));

  // Seta em chevron, com tracos grossos e pontas suaves.
  const int mx = x + BTN_W / 2;
  const int my = BTN_Y + BTN_H / 2;
  const int dir = left ? -1 : 1;
  const uint16_t arrow = pressed ? C_PANEL : color;
  const uint16_t shine = blend(arrow, C_TEXT, pressed ? 0 : 40);
  const int tipX = mx + dir * 12;
  const int outX = mx - dir * 5;
  const int endX = mx - dir * 11;
  const int inX = mx - dir * 3;

  fillQuad(tft, tipX, my, outX, my - 13, endX, my - 10, inX, my, arrow);
  fillQuad(tft, inX, my, endX, my + 10, outX, my + 13, tipX, my, arrow);
  // Fio de luz na borda superior da seta.
  tft.drawLine(tipX, my - 1, outX, my - 13, shine);
}

void drawBase() {
  tft.fillScreen(C_BG);
  drawHeader();
  drawCompass(rotor::currentAzimuth());
  drawArrowButton(BTN_CCW_X, true, false);
  drawArrowButton(BTN_CW_X, false, false);
  lastDrawnDirection = 0;
}

void drawCard(const char* title, int y, int height) {
  tft.fillScreen(C_BG);
  tft.fillRoundRect(28, y, 264, height, 12, C_PANEL);
  tft.drawRoundRect(28, y, 264, height, 12, C_CYAN);
  tft.setTextColor(C_CYAN, C_PANEL);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(title, 160, y + 25, uiFont(3));
}

void drawMenuItem(int y, const char* text, uint16_t color) {
  tft.fillRoundRect(48, y, 224, 26, 7, color);
  tft.setTextColor(C_BG, color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, 160, y + 13, uiFont(2));
}

/** Linha de menu com valor atual à direita (idioma, brilho). */
void drawMenuRow(int y, const char* label, const char* value, uint16_t color) {
  const uint16_t bg = blend(C_PANEL, color, 22);
  tft.fillRoundRect(48, y, 224, 26, 7, bg);
  tft.drawRoundRect(48, y, 224, 26, 7, color);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(color, bg);
  tft.drawString(label, 58, y + 13, uiFont(2));
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(C_TEXT, bg);
  tft.drawString(value, 262, y + 13, uiFont(2));
}

void drawSmallButton(int x, int y, int w, const char* label, uint16_t color) {
  tft.fillRoundRect(x, y, w, 30, 8, color);
  tft.setTextColor(C_BG, color);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + 15, uiFont(3));
}

// Posições das linhas do menu de configurações.
constexpr int kMenuCalibrate = 72;
constexpr int kMenuReset = 99;
constexpr int kMenuLanguage = 126;
constexpr int kMenuBrightness = 153;
constexpr int kMenuAbout = 180;
constexpr int kMenuBack = 207;

void drawConfigMenu() {
  drawCard(tr(T_CONFIG), 32, 206);
  drawMenuItem(kMenuCalibrate, tr(T_CALIBRATE), C_CYAN);
  drawMenuItem(kMenuReset, tr(T_RESET_WIFI), C_RED);
  drawMenuRow(kMenuLanguage, tr(T_LANGUAGE),
              settings::get().language == settings::kLanguageEn ? "EN" : "PT-BR",
              C_TEAL);
  char bri[8];
  snprintf(bri, sizeof(bri), "%d%%", settings::get().brightness);
  drawMenuRow(kMenuBrightness, tr(T_BRIGHTNESS), bri, C_TEAL);
  drawMenuItem(kMenuAbout, tr(T_ABOUT), C_TEAL);
  drawMenuItem(kMenuBack, tr(T_BACK), C_MUTED);
}

void drawBrightness() {
  tft.fillScreen(C_BG);
  tft.fillRoundRect(28, 32, 264, 200, 12, C_PANEL);
  tft.drawRoundRect(28, 32, 264, 200, 12, C_CYAN);

  tft.setTextColor(C_CYAN, C_PANEL);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(tr(T_BRIGHTNESS), 160, 56, uiFont(3));

  const uint8_t value = settings::get().brightness;
  char buf[8];
  snprintf(buf, sizeof(buf), "%d%%", value);
  tft.setTextColor(C_TEXT, C_PANEL);
  tft.drawString(buf, 160, 95, uiFont(4));

  const int barX = 48, barY = 128, barW = 224, barH = 16;
  tft.fillRoundRect(barX, barY, barW, barH, 8, blend(C_PANEL, C_TEXT, 18));
  const int filled = (barW - 4) * value / 100;
  if (filled > 0) {
    tft.fillRoundRect(barX + 2, barY + 2, filled, barH - 4, 6, C_CYAN);
  }

  drawSmallButton(48, 158, 104, "-", C_TEAL);
  drawSmallButton(168, 158, 104, "+", C_TEAL);
  drawMenuItem(198, tr(T_BACK), C_MUTED);
}

void changeBrightness(int delta) {
  const int next = constrain(static_cast<int>(settings::get().brightness) + delta,
                             settings::kBrightnessMin, settings::kBrightnessMax);
  if (next == settings::get().brightness) return;
  settings::get().brightness = static_cast<uint8_t>(next);
  settings::save();
  tft.setBrightness(next);
  drawBrightness();
}

// ---------------------------------------------------------------------------
// Tela de configuração do AP (QR Code)
// ---------------------------------------------------------------------------

void drawApTip();

constexpr int kApNextY = 210;
constexpr int kApTipOkX = 60;
constexpr int kApTipOkY = 150;
constexpr int kApTipOkW = 200;
constexpr int kApTipOkH = 26;

void drawApSetup() {
  tft.fillScreen(C_BG);
  tft.fillRoundRect(8, 4, 304, 232, 14, C_PANEL);
  tft.drawRoundRect(8, 4, 304, 232, 14, C_CYAN);

  tft.setTextColor(C_CYAN, C_PANEL);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(tr(T_WIFI_SETUP), 160, 22, uiFont(3));

  // Módulos escuros sobre fundo claro: alto contraste para qualquer leitor.
  tft.drawBitmap(160 - kApQrWidth / 2, 42, kApQrBits, kApQrWidth, kApQrHeight,
                 C_BG, C_TEXT);

  tft.setTextColor(C_MUTED, C_PANEL);
  tft.drawString(tr(T_CONNECT_AP), 160, 158, uiFont(0));
  tft.setTextColor(C_CYAN, C_PANEL);
  tft.drawString(config::kPortalApName, 160, 178, uiFont(3));
  tft.setTextColor(C_MUTED, C_PANEL);
  tft.drawString(config::kPortalIp, 160, 198, uiFont(1));

  drawMenuItem(kApNextY, tr(T_NEXT), C_TEAL);

  if (apTipVisible) drawApTip();
}

void drawApTip() {
  tft.fillRoundRect(24, 56, 272, 124, 12, C_PANEL);
  tft.drawRoundRect(24, 56, 272, 124, 12, C_CYAN);

  tft.setTextColor(C_CYAN, C_PANEL);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(tr(T_TIP), 160, 80, uiFont(3));
  tft.setTextColor(C_TEXT, C_PANEL);
  tft.drawString(tr(T_TIP_L1), 160, 110, uiFont(2));
  tft.drawString(tr(T_TIP_L2), 160, 132, uiFont(2));

  tft.fillRoundRect(kApTipOkX, kApTipOkY, kApTipOkW, kApTipOkH, 7, C_TEAL);
  tft.setTextColor(C_BG, C_TEAL);
  tft.drawString(tr(T_OK), 160, kApTipOkY + kApTipOkH / 2, uiFont(2));
}

void drawAbout() {
  tft.fillScreen(C_BG);
  tft.fillRoundRect(24, 30, 272, 168, 12, C_PANEL);
  tft.drawRoundRect(24, 30, 272, 168, 12, C_CYAN);

  tft.setTextColor(C_CYAN, C_PANEL);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(tr(T_ABOUT), 160, 52, uiFont(3));

  // Data/ano vêm do NTP quando o ESP está online; fora isso usa a data de
  // compilação, para o painel nunca mostrar campos vazios.
  char dateStr[20] = "--/--/----";
  char yearStr[8] = "----";
  const time_t now = time(nullptr);
  if (now > 1600000000L) {
    struct tm ti{};
    localtime_r(&now, &ti);
    snprintf(dateStr, sizeof(dateStr), "%02d/%02d/%04d", ti.tm_mday,
             ti.tm_mon + 1, ti.tm_year + 1900);
    snprintf(yearStr, sizeof(yearStr), "%04d", ti.tm_year + 1900);
  } else {
    static const char buildDate[] = __DATE__;  // ex.: "Nov 22 2026"
    snprintf(dateStr, sizeof(dateStr), "%.19s", buildDate);
    const size_t len = strlen(buildDate);
    if (len >= 4) snprintf(yearStr, sizeof(yearStr), "%.4s", buildDate + len - 4);
  }

  const settings::Settings& cfg = settings::get();
  const char* labels[5] = {tr(T_CALLSIGN), tr(T_NAME), tr(T_DATE), tr(T_YEAR),
                           tr(T_VERSION)};
  const char* values[5] = {cfg.ownerCallsign.c_str(), cfg.ownerName.c_str(),
                           dateStr, yearStr, config::kFirmwareVersion};
  for (int i = 0; i < 5; ++i) {
    const int y = 74 + i * 23;
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(C_MUTED, C_PANEL);
    tft.drawString(labels[i], 40, y, uiFont(1));
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(C_TEXT, C_PANEL);
    tft.drawString(values[i], 280, y, uiFont(2));
  }

  drawMenuItem(202, tr(T_BACK), C_MUTED);
}

void drawConfirmReset() {
  drawCard(tr(T_RESET_Q), 55, 145);
  tft.setTextColor(C_TEXT, C_PANEL);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(tr(T_ERASE_RESTART), 160, 108, uiFont(1));
  drawMenuItem(125, tr(T_CANCEL), C_MUTED);
  drawMenuItem(162, tr(T_CONFIRM), C_RED);
}

void drawResetPending() {
  tft.fillScreen(C_BG);
  tft.setTextColor(C_CYAN, C_BG);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(tr(T_RESTARTING), 160, 110, uiFont(3));
}

// ---------------------------------------------------------------------------
// Touch
// ---------------------------------------------------------------------------

/** Leitura convertida e com debounce (duas leituras coerentes). */
bool readTouch(int16_t& x, int16_t& y) {
  int16_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;
  if (tft.getTouch(&x1, &y1) == 0) return false;
  delay(5);
  if (tft.getTouch(&x2, &y2) == 0) return false;
  if (abs(x1 - x2) > 12 || abs(y1 - y2) > 12) return false;
  x = static_cast<int16_t>((x1 + x2) / 2);
  y = static_cast<int16_t>((y1 + y2) / 2);
  return true;
}

constexpr int CAL_X0 = 24;
constexpr int CAL_X1 = 296;
constexpr int CAL_Y0 = 30;
constexpr int CAL_Y1 = 210;
constexpr unsigned long kCalPointTimeoutMs = 12000UL;

/** Espera um toque contínuo e devolve a média das coordenadas brutas. */
bool readStableRaw(lgfx::touch_point_t& out) {
  lgfx::touch_point_t last{};
  lgfx::touch_point_t tp{};
  int stable = 0;
  const unsigned long start = millis();

  while (stable < 8) {
    if (tft.getTouchRaw(&tp) != 0) {
      if (stable > 0 && abs(tp.x - last.x) <= 40 && abs(tp.y - last.y) <= 40) {
        ++stable;
      } else {
        stable = 1;
      }
      last = tp;
    } else {
      stable = 0;
    }
    if (millis() - start > kCalPointTimeoutMs) return false;
    delay(25);
  }

  long sx = 0, sy = 0;
  int samples = 0;
  for (int i = 0; i < 8; ++i) {
    if (tft.getTouchRaw(&tp) != 0) {
      sx += tp.x;
      sy += tp.y;
      ++samples;
    }
    delay(15);
  }
  if (samples == 0) return false;

  out.x = static_cast<int16_t>(sx / samples);
  out.y = static_cast<int16_t>(sy / samples);

  const unsigned long releaseStart = millis();
  while (tft.getTouchRaw(&tp) != 0 && millis() - releaseStart < 1500UL) delay(5);
  return true;
}

void drawCalibrationScreen(int step) {
  tft.fillScreen(C_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(C_TEXT, C_BG);
  tft.drawString(tr(T_CAL_TOUCH), 160, 62, uiFont(3));

  char buffer[40];
  snprintf(buffer, sizeof(buffer), tr(T_CAL_POINT), step + 1);
  tft.setTextColor(C_MUTED, C_BG);
  tft.drawString(buffer, 160, 178, uiFont(1));
  tft.drawString(tr(T_CAL_HOLD), 160, 198, uiFont(0));

  static const int16_t targetX[4] = {CAL_X0, CAL_X1, CAL_X1, CAL_X0};
  static const int16_t targetY[4] = {CAL_Y0, CAL_Y0, CAL_Y1, CAL_Y1};
  for (int i = 0; i < 4; ++i) {
    const int px = targetX[i];
    const int py = targetY[i];
    const uint16_t color = (i == step) ? C_CYAN : C_MUTED;
    tft.drawCircle(px, py, 14, color);
    tft.drawLine(px - 22, py, px - 17, py, color);
    tft.drawLine(px + 17, py, px + 22, py, color);
    tft.drawLine(px, py - 22, px, py - 17, color);
    tft.drawLine(px, py + 17, px, py + 22, color);
  }
}

/** Aguarda o dedo sair da tela antes de devolver o controle ao loop. */
bool waitTouchRelease(unsigned long timeoutMs = 2500UL) {
  lgfx::touch_point_t tp{};
  const unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (tft.getTouchRaw(&tp) == 0) return true;
    delay(20);
  }
  return false;
}

/** Encerra a calibração (sucesso ou falha) com estado de toque limpo. */
void finishCalibration() {
  const bool released = waitTouchRelease();
  // Se o dedo ainda estiver na tela, não pode virar "novo toque" no loop.
  previousTouch = !released;
  drawBase();
}

/**
 * Calibração na horizontal (320x240).
 *
 * Coleta o bruto do XPT2046 em 4 alvos visíveis, ajusta uma transformação
 * afim (bruto = f(tela)) e entrega ao LovyanGFX os 4 pontos brutos
 * correspondentes aos cantos exatos da orientação 0 do painel. A partir daí,
 * o próprio LovyanGFX converte para coordenadas da tela em setRotation(1).
 *
 * A ordem exigida por setCalibrate() é:
 *   slot 0 -> (0, 0), slot 1 -> (0, 319),
 *   slot 2 -> (239, 0), slot 3 -> (239, 319)   [orientação 0]
 *
 * Com rotação 1: affine_x = 239 - sy, affine_y = sx. Portanto:
 *   slot 0 -> tela (0, 239),   slot 1 -> tela (319, 239),
 *   slot 2 -> tela (0, 0),     slot 3 -> tela (319, 0).
 */
bool calibrateTouch(bool save) {
  tft.setRotation(1);
  drawCalibrationScreen(0);
  delay(500);

  lgfx::touch_point_t raw[4];

  for (int i = 0; i < 4; ++i) {
    drawCalibrationScreen(i);
    if (!readStableRaw(raw[i])) {
      Serial.println("Touch: calibracao cancelada (timeout)");
      finishCalibration();
      return false;
    }
  }

  // bruto = origem + ru * u + rv * v, com u/v normalizados no retângulo de alvos.
  const float tlX = raw[0].x, tlY = raw[0].y;
  const float trX = raw[1].x, trY = raw[1].y;
  const float brX = raw[2].x, brY = raw[2].y;
  const float blX = raw[3].x, blY = raw[3].y;

  const float ruX = ((trX - tlX) + (brX - blX)) * 0.5f;
  const float rvX = ((blX - tlX) + (brX - trX)) * 0.5f;
  const float ruY = ((trY - tlY) + (brY - blY)) * 0.5f;
  const float rvY = ((blY - tlY) + (brY - trY)) * 0.5f;
  const float oX = (tlX + trX + brX + blX - 2.0f * ruX - 2.0f * rvX) * 0.25f;
  const float oY = (tlY + trY + brY + blY - 2.0f * ruY - 2.0f * rvY) * 0.25f;

  const float det = ruX * rvY - rvX * ruY;
  if (fabsf(det) < 50000.0f) {
    Serial.println("Touch: calibracao invalida (eixos degenerados)");
    finishCalibration();
    return false;
  }

  auto normalized = [](int sx, int sy, float& u, float& v) {
    u = static_cast<float>(sx - CAL_X0) / (CAL_X1 - CAL_X0);
    v = static_cast<float>(sy - CAL_Y0) / (CAL_Y1 - CAL_Y0);
  };
  auto clampRaw = [](float value) -> uint16_t {
    return static_cast<uint16_t>(constrain(lroundf(value), 0L, 4095L));
  };
  auto rawX = [&](int sx, int sy) -> uint16_t {
    float u = 0, v = 0;
    normalized(sx, sy, u, v);
    return clampRaw(oX + ruX * u + rvX * v);
  };
  auto rawY = [&](int sx, int sy) -> uint16_t {
    float u = 0, v = 0;
    normalized(sx, sy, u, v);
    return clampRaw(oY + ruY * u + rvY * v);
  };

  uint16_t orig[8] = {
      rawX(0, 239),   rawY(0, 239),    // slot 0 -> affine (0, 0)
      rawX(319, 239), rawY(319, 239),  // slot 1 -> affine (0, 319)
      rawX(0, 0),     rawY(0, 0),      // slot 2 -> affine (239, 0)
      rawX(319, 0),   rawY(319, 0),    // slot 3 -> affine (239, 319)
  };

  tft.setTouchCalibrate(orig);
  if (save) calibration::save(orig);
  Serial.println("Touch: calibracao horizontal concluida");
  finishCalibration();
  return true;
}

}  // namespace

// ---------------------------------------------------------------------------
// API pública
// ---------------------------------------------------------------------------

void init() {
  applyPalette();
  tft.init();
  tft.setRotation(1);
  tft.setBrightness(settings::get().brightness);

  tft.writecommand(ILI9341_GAMMASET);  //Gamma curve selected
  tft.writedata(2);
  delay(120);
  tft.writecommand(ILI9341_GAMMASET);  //Gamma curve selected
  tft.writedata(1);

  displayFontInit();

  compassSprite.setColorDepth(16);
  compassSpriteOk = (compassSprite.createSprite(COMPASS_BOX, COMPASS_BOX) != nullptr);
  Serial.printf("Display: %dx%d rot=%d, sprite=%s\n", tft.width(), tft.height(),
                tft.getRotation(), compassSpriteOk ? "ok" : "indireto");

  // Descarta leituras instáveis logo após init() do XPT2046.
  for (int i = 0; i < 10; ++i) {
    int16_t x = 0, y = 0;
    tft.getTouch(&x, &y);
    delay(40);
  }

  // Só calibra na primeira vez (sem dados salvos). Nos boots seguintes a
  // calibração da flash é aplicada diretamente; o menu CFG permite refazê-la.
  calibration::init();
  if (calibration::isValid()) {
    uint16_t params[8] = {0};
    const uint16_t* saved = calibration::params();
    for (int i = 0; i < 8; ++i) params[i] = saved[i];
    tft.setTouchCalibrate(params);
    Serial.println("Touch: calibracao aplicada da flash");
    drawBase();
  } else {
    Serial.println("Touch: primeira calibracao (tela inicial)");
    calibrateTouch(true);
  }
}

Input process() {
  Input result;
  int16_t tx = 0, ty = 0;
  const bool touched = readTouch(tx, ty);
  const bool newTouch = touched && !previousTouch;
  previousTouch = touched;

  if (screen == Screen::AP_SETUP) {
    result.rotorBlocked = true;
    if (newTouch) {
      if (apTipVisible) {
        if (inside(tx, ty, kApTipOkX, kApTipOkY, kApTipOkW, kApTipOkH)) {
          apTipVisible = false;
          drawApSetup();
        }
      } else if (inside(tx, ty, 48, kApNextY, 224, 26)) {
        apTipVisible = true;
        drawApSetup();
      }
    }
    return result;
  }

  if (screen == Screen::ABOUT) {
    result.rotorBlocked = true;
    if (newTouch && (inside(tx, ty, 48, 202, 224, 26) ||
                     !inside(tx, ty, 24, 30, 272, 168))) {
      screen = Screen::CONFIG;
      drawConfigMenu();
    }
    return result;
  }

  if (screen == Screen::BRIGHTNESS) {
    result.rotorBlocked = true;
    if (newTouch) {
      if (inside(tx, ty, 48, 158, 104, 30)) {
        changeBrightness(-10);
      } else if (inside(tx, ty, 168, 158, 104, 30)) {
        changeBrightness(10);
      } else if (inside(tx, ty, 48, 198, 224, 26) ||
                 !inside(tx, ty, 28, 32, 264, 200)) {
        screen = Screen::CONFIG;
        drawConfigMenu();
      }
    }
    return result;
  }

  if (screen == Screen::CONFIG) {
    result.rotorBlocked = true;
    if (newTouch) {
      if (inside(tx, ty, 48, kMenuCalibrate, 224, 26)) {
        screen = Screen::MAIN;
        calibrateTouch(true);
      } else if (inside(tx, ty, 48, kMenuReset, 224, 26)) {
        screen = Screen::CONFIRM_RESET;
        drawConfirmReset();
      } else if (inside(tx, ty, 48, kMenuLanguage, 224, 26)) {
        settings::get().language =
            settings::get().language == settings::kLanguageEn
                ? settings::kLanguagePtBr
                : settings::kLanguageEn;
        settings::save();
        drawConfigMenu();
      } else if (inside(tx, ty, 48, kMenuBrightness, 224, 26)) {
        screen = Screen::BRIGHTNESS;
        drawBrightness();
      } else if (inside(tx, ty, 48, kMenuAbout, 224, 26)) {
        screen = Screen::ABOUT;
        drawAbout();
      } else if (inside(tx, ty, 48, kMenuBack, 224, 26) ||
                 !inside(tx, ty, 28, 32, 264, 206)) {
        screen = Screen::MAIN;
        drawBase();
      }
    }
    return result;
  }

  if (screen == Screen::CONFIRM_RESET) {
    result.rotorBlocked = true;
    if (newTouch) {
      if (inside(tx, ty, 48, 125, 224, 32)) {
        screen = Screen::MAIN;
        drawBase();
      } else if (inside(tx, ty, 48, 162, 224, 32)) {
        screen = Screen::MAIN;
        drawResetPending();
        result.resetNetwork = true;
      }
    }
    return result;
  }

  if (newTouch && inside(tx, ty, CFG_X, CFG_Y, CFG_W, CFG_H)) {
    screen = Screen::CONFIG;
    drawConfigMenu();
    result.rotorBlocked = true;
    return result;
  }

  if (touched) {
    if (inside(tx, ty, BTN_CCW_X, BTN_Y, BTN_W, BTN_H)) {
      result.direction = -1;
    } else if (inside(tx, ty, BTN_CW_X, BTN_Y, BTN_W, BTN_H)) {
      result.direction = 1;
    }
  }
  return result;
}

void setDirection(int8_t direction) {
  if (screen != Screen::MAIN) {
    lastDrawnDirection = 0;
    return;
  }
  if (direction == lastDrawnDirection) return;
  drawArrowButton(BTN_CCW_X, true, direction < 0);
  drawArrowButton(BTN_CW_X, false, direction > 0);
  lastDrawnDirection = direction;
}

void updateDisplay(int azimuth) {
  static float filter = static_cast<float>(azimuth);
  static int lastDrawn = -1;
  static unsigned long lastMs = 0;

  filter += 0.28f * (static_cast<float>(azimuth) - filter);
  const int filtered = constrain(static_cast<int>(lroundf(filter)), 0, 360);
  rotor::updateDisplayedAzimuth(filtered);

  if (screen != Screen::MAIN) return;
  const unsigned long now = millis();
  if (filtered != lastDrawn && now - lastMs >= 80UL) {
    drawCompass(filtered);
    lastDrawn = filtered;
    lastMs = now;
  }
}

void redrawCurrentScreen() {
  switch (screen) {
    case Screen::CONFIG:
      drawConfigMenu();
      break;
    case Screen::ABOUT:
      drawAbout();
      break;
    case Screen::BRIGHTNESS:
      drawBrightness();
      break;
    case Screen::AP_SETUP:
      drawApSetup();
      break;
    case Screen::CONFIRM_RESET:
      drawConfirmReset();
      break;
    default:
      drawBase();
      break;
  }
}

void refreshHeaderIfChanged() {
  static uint8_t lastPalette = 255;
  const uint8_t palette = settings::get().palette;
  if (palette != lastPalette) {
    lastPalette = palette;
    applyPalette();
    redrawCurrentScreen();
    return;
  }
  if (screen != Screen::MAIN) {
    settings::consumeChanged();
    return;
  }
  if (settings::consumeChanged()) drawHeader();
}

void setApSetupMode(bool active) {
  if (active == apSetupMode) return;
  apSetupMode = active;
  apTipVisible = false;
  if (active) {
    screen = Screen::AP_SETUP;
    drawApSetup();
  } else {
    screen = Screen::MAIN;
    drawBase();
  }
}

void restoreBase() {
  screen = Screen::MAIN;
  drawBase();
}

}  // namespace hmi
