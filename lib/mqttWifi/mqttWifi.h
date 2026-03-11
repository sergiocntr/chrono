#pragma once
#ifdef ESP8266_BUILD
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#define httpUpdate ESPhttpUpdate

#elif ESP32_BUILD
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
#endif

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <impostazioni.h>
#include <topic.h>
#include <myIP.h>
#include <password.h>
//#include "mqttWifiMessages.h"

namespace mqttWifi
{
  extern PubSubClient client;

  // ========== VARIABILI DI STATO ==========

  // Forward declarations
  void adessoDormo(uint8_t mode, MotivoSpegnimento motivo);
  bool sottoscriviTopic();

  // ========== FUNZIONE LOG MOTIVO ==========
  void logMotivoSpegnimento(MotivoSpegnimento motivo);

  // ========== GESTIONE MODALITÀ RIPOSO ==========
  void adessoDormo(uint8_t mode, MotivoSpegnimento motivo);

  // ========== SETUP INIZIALE ==========
  void setupWifi();

  // ========== CONNESSIONE WIFI ==========
  bool connectWifi();
  // ========== CONNESSIONE MQTT ==========
  bool connectMqtt();

  // ========== SOTTOSCRIZIONE TOPIC ==========
  bool sottoscriviTopic();

    // ========== GESTIONE PUBBLICAZIONE ==========
  bool publish(const char *topic, const char *message, bool retained = false);

  // ========== GESTIONE PRINCIPALE (DA CHIAMARE NEL LOOP) ==========
  MotivoSpegnimento gestisciConnessione();

  // ========== SETUP COMPLETO (DA CHIAMARE IN setup()) ==========
  MotivoSpegnimento setupCompleto();

} // namespace mqttWifi