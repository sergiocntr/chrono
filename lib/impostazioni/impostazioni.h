#pragma once
#include <Arduino.h>
#include <Nextion.h>
#include <topic/topic.h>
#include "myIP/myIP.h"
#include "password/password.h"

//#define versione
extern const char* mqttId;
struct tempStr{
  float t;
  float h;
  uint8_t confort;
};
extern tempStr myTemp;

enum Tende {
    TENDA_SALOTTO,    // 0
    TENDA_LEO,        // 1
    TAPPA_SALOTTO,   // 2
    TAPPA_LEO,        // 3 
    TAPPA_CAMERA      // 4
};

// Enum per i comandi delle tende
enum ComandoTende {
    CHIUDI,        // 0
    APRI,          // 1
    APRI_PARZIALE  // 2
};
extern ComandoTende comandoTenda;

// ========== ENUM MOTIVI SPEGNIMENTO ==========
  enum MotivoSpegnimento
  {
    PUBLISH_FALLITO = 0,
    COMANDO_SYSTEM_TOPIC = 1,
    WIFI_TIMEOUT_GESTIONE = 2,
    MQTT_TIMEOUT_GESTIONE = 3,
    WIFI_FALLITO_SETUP = 4,
    MQTT_FALLITO_RISVEGLIO = 5,
    WIFI_FALLITO_RISVEGLIO = 6,
    NEXTION_SETUP_FAILED = 7,
    SHUTDOWN_FROM_MQTT = 8  
  };
extern NexTouch *nex_listen_list[];
extern uint8_t db_array_value[4];
extern NexText Nset_temp;
extern NexText Ntcurr;
extern NexText Nout_temp;
extern NexCrop Nwater_on;
extern NexText Nout_hum;
extern NexText Nin_hum;
extern NexText Ncurr_hour;
extern NexText Ncurr_water_temp;
extern NexText Nday;
extern NexButton Nb_up;
extern NexButton Nb_down;
extern NexCrop Nrisc_on;
extern NexCrop Nalarm;
