#include <Arduino.h>
#include <WiFiManager.h>

#include <ctime>
#include <vector>

#include "config.h"
#include "hardware/motor.h"
#include "services/calibration.h"
#include "services/rotor_state.h"
#include "services/settings.h"
#include "services/web_portal.h"
#include "ui/hmi.h"

namespace {

constexpr int kAdcMin = 100;   // leitura do potenciômetro em 0 graus
constexpr int kAdcMax = 3900;  // leitura do potenciômetro em 360 graus

WiFiManager wm;

bool waitingForWifi = false;
unsigned long wifiWaitStart = 0;
unsigned long lastReconnectAttempt = 0;
bool wasConnected = false;
unsigned long wifiLostSince = 0;
bool wifiLossReported = false;

void startPortal() {
  wm.setConfigPortalTimeout(0);
  wm.setConfigPortalBlocking(false);  // touch continua respondendo no modo AP
  wm.setConnectTimeout(10);           // falha rápido e sobe o AP em 10 s
  wm.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1),
                         IPAddress(255, 255, 255, 0));
  wm.setHostname(config::kPortalHostname);
  wm.setTitle("Rotor CYD");
  wm.setCustomHeadElement(web::portalHeadHtml());
  wm.setCustomMenuHTML(web::portalMenuHtml());

  std::vector<const char*> menu = {"wifi", "custom", "info", "update", "sep", "restart"};
  wm.setMenu(menu);
  wm.setWebServerCallback([]() { web::registerRoutes(wm); });
}

int readAzimuth() {
  long sum = 0;
  for (int i = 0; i < 10; ++i) sum += analogRead(config::kPinPotRotor);
  const int value = static_cast<int>(sum / 10);
  return constrain(map(value, kAdcMin, kAdcMax, 0, 360), 0, 360);
}

void handleNetworkSetup() {
  String ssid;
  String password;
  if (web::consumeWifiConfig(ssid, password)) {
    Serial.printf("Conectando na rede %s...\n", ssid.c_str());
    WiFi.persistent(true);
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.persistent(false);
    waitingForWifi = true;
    wifiWaitStart = millis();
  }

  if (!waitingForWifi) return;

  if (WiFi.status() == WL_CONNECTED) {
    waitingForWifi = false;
    if (wm.getConfigPortalActive()) wm.stopConfigPortal();
    wm.startWebPortal();
    Serial.printf("Web: http://%s\n", WiFi.localIP().toString().c_str());
  } else if (millis() - wifiWaitStart > 20000UL) {
    waitingForWifi = false;
    Serial.printf("Falha ao conectar na rede %s\n", ssid.c_str());
    hmi::showWifiAlert(ssid.c_str(), false);
  }
}

/** Avisa quando a rede configurada deixa de responder. */
void handleWifiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    if (wifiLossReported) hmi::clearWifiAlert();
    wasConnected = true;
    wifiLostSince = 0;
    wifiLossReported = false;
    return;
  }
  // O portal AP e a conexão em andamento já explicam a situação na tela.
  if (!wasConnected || wm.getConfigPortalActive() || waitingForWifi) return;
  if (wifiLostSince == 0) wifiLostSince = millis();
  if (wifiLossReported || millis() - wifiLostSince < 10000UL) return;

  wifiLossReported = true;
  String ssid = WiFi.SSID();
  if (!ssid.length()) ssid = wm.getWiFiSSID();
  Serial.printf("Aviso: conexao Wi-Fi perdida (%s)\n", ssid.c_str());
  hmi::showWifiAlert(ssid.length() ? ssid.c_str() : "Wi-Fi", true);
}

void handleReconnect() {
  if (wm.getConfigPortalActive() || waitingForWifi) return;
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - lastReconnectAttempt < 15000UL) return;

  lastReconnectAttempt = millis();
  if (WiFi.SSID().length() > 0) WiFi.begin();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(150);

  motor::init();
  settings::init();
  hmi::init();

  startPortal();
  const bool connected = wm.autoConnect(config::kPortalApName);
  if (connected) {
    wasConnected = true;
    // Hora real para o quadro "Sobre" (com fallback para a data de compilação).
    setenv("TZ", "BRT3", 1);
    tzset();
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    wm.startWebPortal();
    Serial.printf("Web: http://%s.local  ou  http://%s\n", config::kPortalHostname,
                  WiFi.localIP().toString().c_str());
  } else {
    Serial.printf("Portal: http://%s\n", config::kPortalIp);
    const String configured = wm.getWiFiSSID();
    if (configured.length()) {
      Serial.printf("Aviso: falha ao conectar na rede salva %s\n",
                    configured.c_str());
      hmi::showWifiAlert(configured.c_str(), false);
    }
  }
}

void loop() {
  rotor::updateAzimuth(readAzimuth());

  // Sem Wi-Fi, o portal AP do WiFiManager ganha a tela com o QR de conexão.
  hmi::setApSetupMode(wm.getConfigPortalActive());

  const hmi::Input input = hmi::process();

  if (input.resetNetwork) {
    motor::allStop();
    Serial.println("Reset de rede solicitado pelo CFG");
    calibration::clear();
    wm.resetSettings();
    delay(200);
    ESP.restart();
    return;
  }

  if (input.openWifiSetup && !wm.getConfigPortalActive()) {
    Serial.println("Abrindo portal AP para trocar o Wi-Fi");
    wm.startConfigPortal(config::kPortalApName);
  }

  // Consome o watchdog mesmo com o menu aberto, para não deixar comando pendente.
  const int8_t webDirection = rotor::webCommand();
  const int8_t direction = input.rotorBlocked
                               ? 0
                               : (input.direction != 0 ? input.direction : webDirection);

  motor::set(direction);
  hmi::setDirection(direction);
  hmi::refreshHeaderIfChanged();
  hmi::updateDisplay(rotor::currentAzimuth());

  if (wm.getWebPortalActive() || wm.getConfigPortalActive()) wm.process();

  handleNetworkSetup();
  handleReconnect();
  handleWifiStatus();

  delay(20);
}
