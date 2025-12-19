#include <Arduino.h>
// Include condizionali basati sulla piattaforma
#ifdef ESP8266_BUILD
  #include <ESP8266WiFi.h>
  #include <ESP8266httpUpdate.h>
  #include <IRremoteESP8266.h>
  #include <IRrecv.h>
  #include <IRutils.h>
  
  //ESP8266HTTPUpdateServer httpUpdater;
  #define nextion Serial
  
#elif ESP32_BUILD
  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <HTTPUpdateServer.h>
  #include <WebServer.h>
  #include <IRremote.h>
  
  WebServer server(80);
  HTTPUpdateServer httpUpdater;
  #define nextion Serial1
  
#endif
#include <DHT.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Int64String.h>

#include <impostazioni.h>
#include "nex.h"
#include "mqttWifi.h"
#include "irnextion.h"
#include "temp.h"
uint32_t wifi_initiate =0;
uint32_t wifi_check_time=60000L;
uint32_t sleep_time = 300000L;

//char buffer[15]={0};
void blinkLed(uint8_t volte);
//void sendData();

void irRoutine();

