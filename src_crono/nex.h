#pragma once
#include <impostazioni.h>
#include "topic.h"
#include "FS.h"
#include "SPI.h" //package builtin configuration file
#include "SD.h"  //package builtin configuration file
#include "mqttWifi.h"
#include <ArduinoJson.h>
namespace nexchr
{

  uint8_t toggle_button(int value)
  {
    if (db_array_value[value] == 1)
    {
      db_array_value[value] = 0;
      return 0;
    }
    else
    {
      db_array_value[value] = 1;
      return 1;
    }
  }

  void update_buttons()
  {
    Nrisc_on.setPic(db_array_value[1]);
    Nwater_on.setPic(db_array_value[2]);
    Nalarm.setPic(db_array_value[3]);
  }

  void Nrisc_onPushCallback(void *ptr)
  {
    uint8_t number = toggle_button(1);
    Nrisc_on.setPic(number);

    // msg.topic = riscaldaTopic;
    if (number == 0)
    {
      // msg.msg = "0";
      mqttWifi::publish(riscaldaTopic, "0");
    }
    else
    {
      mqttWifi::publish(riscaldaTopic, "1");
    }
  }

  void Nb_upPushCallback(void *ptr)
  {
    comandoTenda = APRI_PARZIALE; // Imposta il comando attuale

    Tende tendeTargets[] = {TAPPA_SALOTTO, TAPPA_LEO, TAPPA_CAMERA};

    mqttWifi::sendTende(tendeTargets, sizeof(tendeTargets) / sizeof(tendeTargets[0]), comandoTenda,80);
  }

  void Nb_downPushCallback(void *ptr)
  {

    Tende tendeTargets[] = {TENDA_SALOTTO, TENDA_LEO, TAPPA_SALOTTO, TAPPA_LEO, TAPPA_CAMERA};

    comandoTenda = CHIUDI; // Imposta il comando attuale
                           // Add values in the document
    
  mqttWifi::sendTende(tendeTargets, sizeof(tendeTargets) / sizeof(tendeTargets[0]), comandoTenda,0);
  }

  void Nwater_onPushCallback(void *ptr)
  {
    uint32_t number = toggle_button(2);
    Nwater_on.setPic(number);

    if (number == 0)
    {
      // msg.msg = "0";
      mqttWifi::publish(acquaTopic, "0");
    }
    else
    {
      mqttWifi::publish(acquaTopic, "1");
    }
  }
  void Nb_AlarmCallback(void *ptr)
  {
    uint32_t number = toggle_button(3);

    Nalarm.setPic(number);

    if (number == 0)
    {
      // msg.msg = "0";
      mqttWifi::publish(teleTopic, "spegni");
    }
  }
  void nex_routines()
  {
    nexInit();
    delay(10);
    sendCommand("dim=20");
    Nset_temp.setText("");
    Ntcurr.setText("");
    Nout_temp.setText("");
    Nout_hum.setText("");
    Nin_hum.setText("");
    Ncurr_hour.setText("");
    Ncurr_water_temp.setText("");
    Nday.setText("");
    delay(10);
    Nrisc_on.attachPush(Nrisc_onPushCallback);
    Nwater_on.attachPush(Nwater_onPushCallback);
    Nalarm.attachPush(Nb_AlarmCallback);
    Nb_up.attachPush(Nb_upPushCallback);
    Nb_down.attachPush(Nb_downPushCallback);
    update_buttons();
    sendCommand("sleep=0");
    delay(10);
  }
}
