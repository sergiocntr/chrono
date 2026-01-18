#pragma once
#include <Arduino.h>
#include <Nextion.h>


//#define DEBUG_CHRONO

/**
 * Define logSerial for the output of debug messages.
 */
#define logSerial Serial
#ifdef DEBUG_CHRONO
  #define logSerialPrint(a)        logSerial.print(a)
  #define logSerialPrintln(a)      logSerial.println(a)
  #define logSerialPrintf(...)     logSerial.printf(__VA_ARGS__)
  #define logSerialBegin(a)        logSerial.begin(a)
#else
  #define logSerialPrint(a)        do{}while(0)
  #define logSerialPrintln(a)      do{}while(0)
  #define logSerialPrintf(...)     do{}while(0)
  #define logSerialBegin(a)        do{}while(0)
#endif

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
    WIFI_TIMEOUT_CONNESSIONE = 2,
    MQTT_TIMEOUT_CONNESSIONE = 3,
    WIFI_FALLITO_SETUP = 4,
    MQTT_FALLITO_RISVEGLIO = 5,
    WIFI_FALLITO_RISVEGLIO = 6,
    NEXTION_SETUP_FAILED = 7,
    DHT_SETUP_FAILED = 8,
    SHUTDOWN_FROM_MQTT = 255  
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
