#include "temp.h"
#include "mqttWifiMessages.h"
namespace tempDHT {
// Configurazione Pin e Variabili
DHTesp dht;
static uint8_t volteTemp = 0;
// Ticker tempTicker;
// TaskHandle_t tempTaskHandle = NULL;
bool setupTemp() {
  dht.setup(DHT_PIN, DHTesp::DHT22);
  
  if (dht.getStatus() != 0) {
    LOG_ERROR("[DHT] error status: %s\n",  String(dht.getStatusString()));
    return false;
  }else{
    LOG_VERBOSE("[DHT] status: %s\n",  String(dht.getStatusString()));
  }
  // Inizializzazione variabili media
  volteTemp = 0;
  myTemp.t = 0;
  myTemp.h = 0;
  return true;
  // // Creazione del Task (Stack dimensionato per ESP32-C3)
  // xTaskCreate(
  //   tempTask,
  //   "tempTask",
  //   2560,
  //   NULL,
  //   1,
  //   &tempTaskHandle
  // );

  // if (tempTaskHandle != NULL) {
  //   // Avvia il ticker ogni 240 secondi (ogni 4 minuti e trasmsmione ongi 16
  //   minuti!) tempTicker.attach(240, triggerTask);
  // }
}

// void triggerTask() {
//   if (tempTaskHandle != NULL) {
//     xTaskResumeFromISR(tempTaskHandle);
//   }
// }

// void tempTask(void *pvParameters) {
//   for(;;) {
//     getLocalTemp(); // Esegue la logica di lettura
//     vTaskSuspend(NULL); // Torna a dormire fino al prossimo trigger
//   }
// }

// Firma originale mantenuta
void getLocalTemp() {
  uint8_t comfortMask = 0;
  TempAndHumidity newValues = dht.getTempAndHumidity();

  if (dht.getStatus() != 0) {
    LOG_ERROR("[DHT] Error reading DHT!\n");
    mqttWifi::publish(logTopic, "Error reading DHT");
    return;
  }

  // Accumulo per media
  myTemp.t += newValues.temperature;
  myTemp.h += newValues.humidity;
  volteTemp++;

  if (volteTemp >= 4) {
    ComfortState cf;
    dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);
    comfortMask = (1 << (uint8_t)cf);
    // Calcolo Comfort Bitmask (Risparmio dati)
    // Bit 0: OK, Bit 1: Too Hot, Bit 2: Too Cold, Bit 3: Too Dry, Bit 4: Too
    // Humid Qui mqttWifi::sendData() leggerà myTemp.t/h e comfortMask
    myTemp.confort = comfortMask;
    mqttWifi::sendData();
    volteTemp = 0;
    // Reset solo se pubblicazione riuscita
    myTemp.h = 0;
    myTemp.t = 0;
    // o subito dopo la chiamata qui.
  }
}
} // namespace tempDHT