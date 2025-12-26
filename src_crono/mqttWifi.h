#pragma once
#ifdef ESP8266_BUILD
  #include <ESP8266WiFi.h>
  #include <ESP8266httpUpdate.h>

  
  //ESP8266HTTPUpdateServer httpUpdater;
  #define nextion Serial
  
#elif ESP32_BUILD
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
//#define nextion Serial1

  
#endif
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <impostazioni.h>

#include "mqttWifiMessages.h" 

namespace mqttWifi
{
  extern PubSubClient client;

 // ========== VARIABILI DI STATO ==========
  extern bool mqttConnesso;
  extern bool inFaseRiposo;
  extern unsigned long timerSenzaConnessione;
  extern const unsigned long INTERVALLO_RIPROVA;
  extern const unsigned long TIMEOUT_WIFI;
  extern const unsigned long TIMEOUT_MQTT;

  // Forward declarations
  void adessoDormo(uint8_t mode, MotivoSpegnimento motivo);
  bool reconnectMqtt();

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
  bool reconnectMqtt();

  // ========== RISVEGLIO DOPO RIPOSO ==========
  void risveglioE_Riconnetti();

  // ========== GESTIONE PRINCIPALE (DA CHIAMARE NEL LOOP) ==========
  void gestisciConnessione();

  // ========== SETUP COMPLETO (DA CHIAMARE IN setup()) ==========
  void setupCompleto();

} // namespace mqttWifi