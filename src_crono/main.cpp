#include <main.h>
/// @brief
/// @param mytime
void smartDelay(unsigned long mytime) {
  uint32_t adesso = millis();
  while ((millis() - adesso) < mytime) {
    mqttWifi::client.loop();
    nexLoop(nex_listen_list);
    delay(200);
  }
}

void irRoutine() {
  if (irrecv.decode(&results)) {
    uint64_t infraredNewValue = results.value;
    switch (infraredNewValue) {
    case monnezza:
      mqttWifi::publish(teleTopic, "monnezza");
      break;
    case spegni:
      mqttWifi::publish(teleTopic, "spegni");
      delay(10);
      logSerialPrintln("[irRoutine] SHUTDOWN_FROM_MQTT!");
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

void setup() {
  delay(500); // inizializzazione generale

#if ESP32_BUILD
  // ===== DISABILITAZIONE BLUETOOTH COMPLETA =====

  // 1. Ferma il Bluetooth Classic
  btStop();

// 2. Deinizializza il controller (livello più basso)
#if CONFIG_BT_ENABLED
  // Rilascia la memoria BT per il WiFi
  esp_bt_mem_release(ESP_BT_MODE_BTDM);
  logSerialPrintln("Bluetooth disabilitato e memoria rilasciata");

#endif
#endif
  bool nexTest = nexchr::nex_routines(); // init Nextion
  logSerialBegin(9600);
  logSerialPrintln("Start");

  mqttWifi::setupCompleto();

  logSerialPrintln("Setup wifi OK");

  // irrecv.enableIRIn();      // IR
  // wifi_initiate = millis(); // timestamp

  if (!nexTest) {
    logSerialPrintln("[SETUP] NEXTION Init faled!");
    mqttWifi::publish(logTopic, "Chrono : Nextion Fail!");
    // mqttWifi::adessoDormo(8, NEXTION_SETUP_FAILED); // entra in sleep se
    // fallisce
  }

  logSerialPrintln("Nextion OK");
  bool dhtTest = tempDHT::setupTemp(); // setup DHT22 e lancio tasker per
                                       // letture ogni 4 minuti
  if (!dhtTest) {
    logSerialPrintln("[SETUP] DHT Init faled!");
    mqttWifi::publish(logTopic, "Chrono : DHT Fail!");
    mqttWifi::adessoDormo(8, DHT_SETUP_FAILED); // entra in sleep se fallisce
  }
  t = millis();
  tempDHT::getLocalTemp();

  logSerialPrintln("End of Setup");
}

void loop() {
  if ((millis() - t) > time_between_sensors_reads) {
    t = millis();
    tempDHT::getLocalTemp();
  }

  mqttWifi::gestisciConnessione();
  smartDelay(10000);
}
