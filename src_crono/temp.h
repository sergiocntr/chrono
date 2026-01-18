#include <Arduino.h>
#include <DHTesp.h>
//#include <Ticker.h>
#include "mqttWifi.h"

namespace tempDHT {
  // Configurazione Pin e Variabili
  extern DHTesp dht;
  //extern Ticker tempTicker;
  //extern TaskHandle_t tempTaskHandle;
  static uint8_t volteTemp;
  // Firma originale mantenuta
  bool setupTemp();
  void getLocalTemp();
  // void tempTask(void *pvParameters);
  // void triggerTask();
}