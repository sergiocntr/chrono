#pragma once
#ifdef ESP8266_BUILD
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

#define nextion Serial

#elif ESP32_BUILD
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "esp_bt.h"
#include "esp_bt_main.h"

#endif
#include "impostazioni.h"
#include "mqttWifi.h" // Include il file con connessioni

namespace mqttWifi
{
  void setCallback();
  // ========== INVIO DATI SENSORI ==========
  void sendData();

  // ========== INVIO COMANDI TENDE ==========


  // ========== AGGIORNAMENTO FIRMWARE ==========
  void checkForUpdates();

  // ========== CALLBACK MQTT ==========
  void callback(char *topic, byte *payload, unsigned int length);

} // namespace mqttWifi