#include <Arduino.h>
#include <impostazioni.h>
// Include condizionali basati sulla piattaforma
#ifdef ESP8266_BUILD
  #include <ESP8266WiFi.h>
  #include <ESP8266httpUpdate.h>
  
#elif ESP32_BUILD
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp
#include <Ticker.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Int64String.h>
#include <topic.h>
#include "myIP.h"
#include "password.h"

#include "nex.h"
#include "mqttWifi.h"
#include "mqttWifiMessages.h" 
#include "irnextion.h"
#include "temp.h"
// uint32_t wifi_initiate =0;
// uint32_t wifi_check_time=60000L;
// uint32_t sleep_time = 300000L;
static uint32_t t ;
uint32_t time_between_sensors_reads = 120000L;
void irRoutine();

