#pragma once
#include <impostazioni.h>
#include "FS.h"
#include "SPI.h" //package builtin configuration file
#include "SD.h"  //package builtin configuration file
#include "mqttWifi.h"
#include "mqttWifiMessages.h"
// #include <ArduinoJson.h>
namespace nexchr
{

  uint8_t toggle_button(int value);

  void update_buttons();
  void Nrisc_onPushCallback(void *ptr);

  void Nb_upPushCallback(void *ptr);

  void Nb_downPushCallback(void *ptr);

  void Nwater_onPushCallback(void *ptr);
  void Nb_AlarmCallback(void *ptr);
  bool nex_routines();
}
