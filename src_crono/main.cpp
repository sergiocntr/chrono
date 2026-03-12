#include <main.h>
#include "NexManager.h"
#include "PageHandlers.h"
#include "mqttWifiMessages.h"
/// @brief
/// @param mytime
MotivoSpegnimento m_wifi_status;
void smartDelay(unsigned long mytime)
{
  uint32_t adesso = millis();
  //uint32_t lastRefresh = 0; // Per controllo più fine del refresh

  while ((millis() - adesso) < mytime)
  {
    // 1. MQTT sempre in ascolto
    mqttWifi::client.loop();

    // 2. Poll Nextion - ora ritorna l'evento
    NexManager::TouchEvent evt = NexManager::poll();
    if (evt.isValid)
    {
      switch (evt.page)
      {
      case 0:
        handleHomePage(evt);
        break;
      case 1:
        handleTendePage(evt);
        break;
      default:
        break;
      }
    }

    // 3. Refresh intelligente (non a ogni ciclo)
    // if (millis() - lastRefresh > 200)
    // { // Aggiorna ogni 200ms max
    //   NexManager::refreshCurrentPage();
    //   lastRefresh = millis();
    // }

    // 4. Delay breve per non bloccare
    delay(10); // 10ms è meglio di 200ms per reattività
  }
}

void irRoutine()
{
  if (irrecv.decode(&results))
  {
    uint64_t infraredNewValue = results.value;
    switch (infraredNewValue)
    {
    case monnezza:
      mqttWifi::publish(teleTopic, "monnezza");
      break;
    case spegni:
      mqttWifi::publish(teleTopic, "spegni");
      delay(10);
      LOG_VERBOSE("[irRoutine] SHUTDOWN_FROM_MQTT!");
      mqttWifi::adessoDormo(8, MotivoSpegnimento::SHUTDOWN_FROM_MQTT);
      break;
    case acquaON:
      mqttWifi::publish(acquaTopic, "1");
      break;
    case eneOff:
      mqttWifi::publish(eneTopic, "0");
      break;
    default:
      String s = int64String(infraredNewValue, HEX, false);
      mqttWifi::publish(iRTopic, s.c_str());
      break;
    }
    yield();
    irrecv.resume(); // Receive the next value
  }
}

void setup()
{
  delay(3000); // inizializzazione generale

#if ESP32_BUILD
  // ===== DISABILITAZIONE BLUETOOTH COMPLETA =====

  // 1. Ferma il Bluetooth Classic
  btStop();

// 2. Deinizializza il controller (livello più basso)
#if CONFIG_BT_ENABLED
  // Rilascia la memoria BT per il WiFi
  esp_bt_mem_release(ESP_BT_MODE_BTDM);
  LOG_VERBOSE("Bluetooth disabilitato e memoria rilasciata");

#endif
#endif
  //bool nexTest = nexchr::nex_routines(); // init Nextion
  NexManager::begin(38400);
  LOG_VERBOSE("Start");
  delay(1000);
  

  m_wifi_status = mqttWifi::setupCompleto();
  if (m_wifi_status != CONN_OK)
{
  NexManager::shutdownNextion(); 
  mqttWifi::adessoDormo(8, m_wifi_status);
}
  
mqttWifi::setCallback(); //************************************************** <---------------------

  LOG_VERBOSE("Setup wifi OK");

  // irrecv.enableIRIn();      // IR
  // wifi_initiate = millis(); // timestamp

  // if (!nexTest)
  // {
  //   LOG_VERBOSE("[SETUP] NEXTION Init faled!");
  //   mqttWifi::publish(logTopic, "Chrono : Nextion Fail!");
  //   // mqttWifi::adessoDormo(8, NEXTION_SETUP_FAILED); // entra in sleep se
  //   // fallisce
  // }

  LOG_VERBOSE("Nextion OK");
  bool dhtTest = tempDHT::setupTemp(); // setup DHT22 e lancio tasker per
                                       // letture ogni 4 minuti
  if (!dhtTest)
  {
    LOG_VERBOSE("[SETUP] DHT Init faled!");
    NexManager::shutdownNextion(); 
    mqttWifi::publish(logTopic, "Chrono : DHT Fail!");
    mqttWifi::adessoDormo(8, DHT_SETUP_FAILED); // entra in sleep se fallisce
  }
  t = millis();
  tempDHT::getLocalTemp();
  NexManager::wakeupNextion();
  NexManager::refreshCurrentPage();
  LOG_VERBOSE("End of Setup");
}

void loop()
{
  if ((millis() - t) > time_between_sensors_reads)
  {
    t = millis();
    tempDHT::getLocalTemp();
  }

  m_wifi_status = mqttWifi::gestisciConnessione();
  
  if (m_wifi_status != CONN_OK)
{
  NexManager::shutdownNextion(); 
  mqttWifi::adessoDormo(8, m_wifi_status);
}
  smartDelay(10000);
}
