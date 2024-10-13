#pragma once
#include <Arduino.h>
#include "Nextion.h"
const uint16_t versione = 148;
const char* mqttId="Chrono";
struct tempStr{
  float t;
  float h;
};
tempStr myTemp;

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
};ComandoTende comandoTenda;

uint8_t db_array_value[4] = {0};
NexText Nset_temp         = NexText(0, 2, "Nset_temp");
NexText Ntcurr            = NexText(0, 3, "Ntcurr");
NexText Nout_temp         = NexText(0, 4, "Nout_temp");
NexCrop Nwater_on         = NexCrop(0, 5, "Nwater_on");
NexText Nout_hum          = NexText(0, 6, "Nout_hum");
NexText Nin_hum           = NexText(0, 7, "Nin_hum");
NexText Ncurr_hour        = NexText(0, 8, "Ncurr_hour");
NexText Ncurr_water_temp  = NexText(0, 11, "Nwater_temp");
NexText Nday              = NexText(0, 12, "Nday");
NexButton Nb_up           = NexButton(0, 9, "Nb_up");
NexButton Nb_down         = NexButton(0, 10, "Nb_down");
NexCrop Nrisc_on          = NexCrop(0, 13, "Nrisc_on");
NexCrop Nalarm            = NexCrop(0, 14, "Nalarm");
NexTouch *nex_listen_list[] ={
  &Nrisc_on,
  &Nwater_on,
  &Nalarm,
  &Nb_up,
  &Nb_down,
  NULL
};
