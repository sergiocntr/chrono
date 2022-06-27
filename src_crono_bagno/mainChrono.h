#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "FS.h"
#include <DallasTemperature.h>
//#include <Int64String.h>
//#include "SoftwareSerial.h"
//#include "EspSaveCrash.h"
//#include "nextion_ser.h"
#include "myIP.h"
#include "password.h"
#include "topic.h"
#include "SPI.h"               //package builtin configuration file
#include "SD.h"               //package builtin configuration file
#include "Nextion.h"
#include "NexxFloat.h"
const uint16_t versione = 2;
uint8_t db_array_value[4] = {0};  //ris 1 acqua 2 alarm 3
//char buffer[15]={0};
boolean AlarmOn = false; // ALLARME ACQUA IN SPEGNIMENTO
boolean CaldOn = false; // ACQUA IN RISCALDAMENTO
boolean AcqOn = false; // E' STATA CHIESTA ACQUA CALDA
boolean RisOn = false; // E' STATA CHIESTA ACQUA CALDA

NexText Nday                = NexText(0, 5, "NtxtDay");
NexxFloat NtxtStanza        = NexxFloat(0, 6, "NtxtStanza");
NexText NtxtAcq             = NexText(0, 4, "NtxtAcq");
NexText NtxtOra             = NexText(0, 3, "NtxtOra");
NexCrop NcrAcq              = NexCrop(0, 2, "NcrAcq");
NexCrop NcrRis              = NexCrop(0, 1, "NcrRis");
//NexCrop NcrAlert            = NexCrop(0, 1, "NcrAlert");
NexTouch *nex_listen_list[] ={
  &NcrRis,
  &NcrAcq,
 // &NcrAlert,
  NULL
};

WiFiClient mywifi;
WiFiClient c;
PubSubClient client(c);

uint8_t mqtt_reconnect_tries=0;
uint8_t mqttOK=0;
uint32_t wifi_initiate =0;

const char* mqttId="Bagno";


const uint8_t ONE_WIRE_BUS = D7;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

void reconnect();
void checkConn();
void callback(char* topic, byte* payload, unsigned int length);
void nex_routines();
void Nwater_onPushCallback(void *ptr);
void Nrisc_onPushCallback(void *ptr);
void Nb_downPushCallback(void *ptr);
void Nb_upPushCallback(void *ptr);
void update_buttons();
uint8_t toggle_button(int value);
void stampaDebug(int8_t intmess);

void smartDelay(unsigned long mytime);
void getLocalTemp();
void blinkLed(uint8_t volte);
void adessoDormo();
void setupWifi();
void checkForUpdates();
void toggle_alarm();
void toggle_acq();
