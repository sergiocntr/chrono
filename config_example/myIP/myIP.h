#pragma once
#ifdef ESP8266_BUILD
#include <ESP8266WiFi.h>
#elif ESP32_BUILD
#include <WiFi.h>
#endif
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress dns1;
extern IPAddress dns2;
///////////////////////////////////////////////////////////
extern IPAddress ipMqtt_server; // MQTT Broker static IP
extern IPAddress ipChrono;      // Node static IP
//////////////////////////////////////////////////////////
extern const char *chronoId;
extern const uint16_t mqtt_port;
