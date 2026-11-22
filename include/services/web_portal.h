#pragma once

#include <Arduino.h>

class WiFiManager;

namespace web {

const char* portalHeadHtml();
const char* portalMenuHtml();
void registerRoutes(WiFiManager& wm);
/** Retorna a rede pendente digitada no portal ( chamado pelo loop principal ). */
bool consumeWifiConfig(String& ssid, String& password);

}  // namespace web
