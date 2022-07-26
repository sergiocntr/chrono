#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <Int64String.h>
#include <impostazioni.h>
#include "nex.h"
#include "mqttWifi.h"
#include "irnextion.h"
#include "temp.h"
uint32_t wifi_initiate =0;
uint32_t wifi_check_time=60000L;

//char buffer[15]={0};
void blinkLed(uint8_t volte);
void sendData();

void irRoutine();

