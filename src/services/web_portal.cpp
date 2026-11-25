#include "services/web_portal.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "config.h"
#include "services/rotor_state.h"
#include "services/settings.h"
#include "web_pages.h"

namespace web {
namespace {

WiFiManager* s_wm = nullptr;
bool s_wifiPending = false;
String s_wifiSsid;
String s_wifiPassword;
bool s_scanActive = false;
unsigned long s_scanStart = 0;
unsigned long s_scanCacheAt = 0;
String s_scanCache;

constexpr unsigned long kScanTimeoutMs = 12000UL;
constexpr unsigned long kScanCacheMs = 5000UL;
constexpr int kMaxScanResults = 24;

String buildScanJson(int16_t count);

WiFiManager::WM_WebServer& srv() { return *s_wm->server; }

void sendJson(int code, JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  srv().sendHeader("Cache-Control", "no-store");
  srv().send(code, "application/json", out);
}

void sendError(int code, const char* msg) {
  JsonDocument doc;
  doc["error"] = msg;
  sendJson(code, doc);
}

void sendOk() {
  JsonDocument doc;
  doc["ok"] = true;
  sendJson(200, doc);
}

void sendPage(const char* body) {
  auto& s = srv();
  s.sendHeader("Cache-Control", "no-store");
  s.setContentLength(CONTENT_LENGTH_UNKNOWN);
  s.send(200, "text/html", "");
  s.sendContent(pages::kShellStart);
  s.sendContent(pages::kHead);
  s.sendContent(pages::kShellBody);
  s.sendContent(body);
  s.sendContent(pages::kShellEnd);
  s.sendContent("");
}

bool parseBody(JsonDocument& doc) {
  if (!srv().hasArg("plain")) return false;
  return deserializeJson(doc, srv().arg("plain")) == DeserializationError::Ok;
}

String field(const JsonDocument& doc, const char* key, size_t max_len) {
  const char* value = doc[key] | "";
  String out(value);
  out.trim();
  if (out.length() > max_len) out = out.substring(0, max_len);
  return out;
}

void handleCss() {
  srv().sendHeader("Cache-Control", "public, max-age=86400");
  srv().send(200, "text/css", pages::kCss);
}

// A tela inicial mostra a bússola e o controle; a navegação já está no topo.
void handleRoot() { sendPage(pages::kControl); }
void handleSettingsPage() { sendPage(pages::kSettings); }
void handleWifiPage() { sendPage(pages::kWifi); }

void handleState() {
  JsonDocument doc;
  doc["azimuth"] = rotor::displayedAzimuth();
  doc["antenna"] = settings::get().antenna;
  doc["callsign"] = settings::get().callsign;
  doc["palette"] = settings::get().palette;
  doc["palette_count"] = settings::kPaletteCount;
  doc["version"] = config::kFirmwareVersion;
  doc["developer_callsign"] = config::kDeveloperCallsign;
  doc["developer_name"] = config::kDeveloperName;
  // commandedDirection() não consome o watchdog do comando (issue corrigida).
  doc["moving"] = rotor::commandedDirection();
  doc["target_active"] = rotor::targetActive();
  doc["target"] = rotor::targetActive() ? rotor::targetAngle() : -1;
  doc["angle_min"] = rotor::kAngleMin;
  doc["angle_max"] = rotor::kAngleMax;
  doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
  doc["wifi_ssid"] = WiFi.SSID();
  doc["wifi_ip"] = WiFi.localIP().toString();
  sendJson(200, doc);
}

void handleSettingsPost() {
  JsonDocument doc;
  if (!parseBody(doc)) return sendError(400, "JSON invalido");
  if (!doc["antenna"].is<const char*>() || !doc["callsign"].is<const char*>()) {
    return sendError(400, "Campos obrigatorios: antenna, callsign");
  }
  auto& cfg = settings::get();
  cfg.antenna = field(doc, "antenna", 24);
  cfg.callsign = field(doc, "callsign", 12);
  if (cfg.antenna.length() == 0 || cfg.callsign.length() == 0) {
    return sendError(400, "Campos nao podem ficar vazios");
  }
  settings::save();
  sendOk();
}

struct ScanResult {
  String ssid;
  int32_t rssi;
  bool secure;
};

String buildScanJson(int16_t count) {
  JsonDocument doc;
  JsonArray networks = doc["networks"].to<JsonArray>();

  if (count > 0) {
    ScanResult found[kMaxScanResults];
    int total = 0;
    for (int i = 0; i < count && total < kMaxScanResults; ++i) {
      const String ssid = WiFi.SSID(i);
      if (ssid.length() == 0) continue;
      bool duplicate = false;
      for (int j = 0; j < total; ++j) {
        if (found[j].ssid == ssid) {
          if (WiFi.RSSI(i) > found[j].rssi) found[j].rssi = WiFi.RSSI(i);
          duplicate = true;
          break;
        }
      }
      if (duplicate) continue;
      found[total].ssid = ssid;
      found[total].rssi = WiFi.RSSI(i);
      found[total].secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
      ++total;
    }

    // Ordena por sinal (mais forte primeiro).
    for (int i = 1; i < total; ++i) {
      const ScanResult item = found[i];
      int j = i - 1;
      while (j >= 0 && found[j].rssi < item.rssi) {
        found[j + 1] = found[j];
        --j;
      }
      found[j + 1] = item;
    }

    for (int i = 0; i < total; ++i) {
      JsonObject net = networks.add<JsonObject>();
      net["ssid"] = found[i].ssid;
      net["rssi"] = found[i].rssi;
      net["secure"] = found[i].secure;
    }
  }

  doc["scanning"] = false;
  String out;
  serializeJson(doc, out);
  return out;
}

/** Varredura assíncrona: a primeira chamada inicia, as seguintes consultam. */
void handleWifiScan() {
  // Serve o último resultado por alguns segundos; evita travar a HTTP
  // enquanto o rádio está ocupado e mantém a lista estável na tela.
  const bool force = srv().hasArg("force");
  if (!force && !s_scanActive && s_scanCache.length() > 0 &&
      millis() - s_scanCacheAt < kScanCacheMs) {
    srv().sendHeader("Cache-Control", "no-store");
    return srv().send(200, "application/json", s_scanCache);
  }

  JsonDocument doc;

  if (!s_scanActive) {
    s_scanCache = "";
    WiFi.scanDelete();
    WiFi.scanNetworks(true /* async */, false /* show_hidden */,
                      false /* passive */, 120 /* ms por canal */);
    s_scanActive = true;
    s_scanStart = millis();
    doc["scanning"] = true;
    return sendJson(200, doc);
  }

  const int16_t count = WiFi.scanComplete();
  if (count == WIFI_SCAN_RUNNING) {
    if (millis() - s_scanStart > kScanTimeoutMs) {
      WiFi.scanDelete();
      s_scanActive = false;
      return sendError(504, "Timeout ao buscar redes");
    }
    doc["scanning"] = true;
    return sendJson(200, doc);
  }

  s_scanActive = false;
  if (count < 0) {
    WiFi.scanDelete();
    return sendError(500, "Falha ao buscar redes");
  }

  s_scanCache = buildScanJson(count);
  s_scanCacheAt = millis();
  WiFi.scanDelete();
  srv().sendHeader("Cache-Control", "no-store");
  srv().send(200, "application/json", s_scanCache);
}

void handlePalettePost() {
  JsonDocument doc;
  if (!parseBody(doc)) return sendError(400, "JSON invalido");
  if (!doc["palette"].is<int>()) return sendError(400, "Campo obrigatorio: palette");
  const int palette = doc["palette"] | 0;
  if (palette < 0 || palette >= settings::kPaletteCount) {
    return sendError(400, "Paleta invalida");
  }
  settings::get().palette = static_cast<uint8_t>(palette);
  settings::save();
  sendOk();
}

void handleRotatePost() {
  JsonDocument doc;
  if (!parseBody(doc)) return sendError(400, "JSON invalido");
  if (!doc["dir"].is<int>()) return sendError(400, "Campo obrigatorio: dir");
  int dir = doc["dir"] | 0;
  if (dir < -1) dir = -1;
  if (dir > 1) dir = 1;
  rotor::setWebCommand(static_cast<int8_t>(dir));
  sendOk();
}

void handlePresetsGet() {
  JsonDocument doc;
  JsonArray list = doc["presets"].to<JsonArray>();
  const uint8_t count = settings::presetCount();
  for (uint8_t i = 0; i < count; ++i) {
    JsonObject item = list.add<JsonObject>();
    item["name"] = settings::presetName(i);
    item["angle"] = settings::presetAngle(i);
  }
  doc["max"] = settings::kMaxPresets;
  sendJson(200, doc);
}

void handlePresetsPost() {
  JsonDocument doc;
  if (!parseBody(doc)) return sendError(400, "JSON invalido");
  const String op = field(doc, "op", 16);
  const uint8_t index = static_cast<uint8_t>(doc["index"] | 0);
  const String name = field(doc, "name", settings::kPresetNameMax);
  const int angle = doc["angle"] | 0;

  bool ok = false;
  if (op == "add") {
    ok = settings::addPreset(name, angle);
  } else if (op == "edit") {
    ok = settings::editPreset(index, name, angle);
  } else if (op == "remove") {
    ok = settings::removePreset(index);
  } else {
    return sendError(400, "Operacao invalida");
  }
  if (!ok) return sendError(400, "Preset invalido ou lista cheia");
  sendOk();
}

void handlePresetGo() {
  JsonDocument doc;
  if (!parseBody(doc)) return sendError(400, "JSON invalido");

  int angle = 0;
  if (doc["angle"].is<int>()) {
    angle = doc["angle"] | 0;
  } else if (doc["index"].is<int>()) {
    const int index = doc["index"] | -1;
    if (index < 0 || index >= settings::presetCount()) {
      return sendError(400, "Preset inexistente");
    }
    angle = settings::presetAngle(index);
  } else {
    return sendError(400, "Campo obrigatorio: index ou angle");
  }

  rotor::setTarget(angle);
  sendOk();
}

void handleWifiPost() {
  JsonDocument doc;
  if (!parseBody(doc)) return sendError(400, "JSON invalido");
  if (!doc["ssid"].is<const char*>()) return sendError(400, "Campo obrigatorio: ssid");
  const char* rawPass = doc["password"] | "";
  s_wifiSsid = field(doc, "ssid", 32);
  s_wifiPassword = String(rawPass);
  if (s_wifiPassword.length() > 64) s_wifiPassword = s_wifiPassword.substring(0, 64);
  if (s_wifiSsid.length() == 0) return sendError(400, "SSID nao pode ficar vazio");
  // A conexao e feita no loop principal, depois que a resposta HTTP foi enviada.
  s_wifiPending = true;
  sendOk();
}

}  // namespace

const char* portalHeadHtml() { return pages::kHead; }
const char* portalMenuHtml() { return pages::kMenu; }

bool consumeWifiConfig(String& ssid, String& password) {
  if (!s_wifiPending) return false;
  ssid = s_wifiSsid;
  password = s_wifiPassword;
  s_wifiPending = false;
  return true;
}

void registerRoutes(WiFiManager& wm) {
  s_wm = &wm;
  auto& s = srv();
  s.on("/pr.css", HTTP_GET, handleCss);
  s.on("/", HTTP_GET, handleRoot);
  s.on("/settings", HTTP_GET, handleSettingsPage);
  s.on("/rede", HTTP_GET, handleWifiPage);
  s.on("/api/state", HTTP_GET, handleState);
  s.on("/api/settings", HTTP_POST, handleSettingsPost);
  s.on("/api/palette", HTTP_POST, handlePalettePost);
  s.on("/api/rotate", HTTP_POST, handleRotatePost);
  s.on("/api/presets", HTTP_GET, handlePresetsGet);
  s.on("/api/presets", HTTP_POST, handlePresetsPost);
  s.on("/api/preset", HTTP_POST, handlePresetGo);
  s.on("/api/wifi", HTTP_POST, handleWifiPost);
  s.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
}

}  // namespace web
