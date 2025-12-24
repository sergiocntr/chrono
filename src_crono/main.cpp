#include <main.h>
void smartDelay(unsigned long mytime)
{
  uint32_t adesso = millis();
  while ((millis() - adesso) < mytime)
  {
    mqttWifi::client.loop();
    nexLoop(nex_listen_list);
    delay(200);
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
      Serial.println("[irRoutine] SHUTDOWN_FROM_MQTT!");
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
  delay(2000); // inizializzazione generale
  // ===== DISABILITAZIONE BLUETOOTH COMPLETA =====
  
  // 1. Ferma il Bluetooth Classic
  btStop();
  
  // 2. Deinizializza il controller (livello più basso)
  #ifdef CONFIG_BT_ENABLED
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED) {
      esp_bt_controller_disable();
    }
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED) {
      esp_bt_controller_deinit();
    }
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
  #endif

  Serial.begin(115200);
  delay(2500);
  Serial.println("Start");

  mqttWifi::setupCompleto();

  Serial.println("Setup wifi OK");

 // irrecv.enableIRIn();      // IR
 // wifi_initiate = millis(); // timestamp

  bool nexTest = nexchr::nex_routines(); // init Nextion
  if (!nexTest)
  {
    Serial.println("[SEUP] NEXTION Init faled!");
    mqttWifi::publish(logTopic, "Chrono : Nextion Fail!");
    mqttWifi::adessoDormo(8, MotivoSpegnimento::NEXTION_SETUP_FAILED); // entra in sleep se fallisce
  }
  Serial.println("Nextion OK");
  tempDHT::setupTemp(); // setup DHT22 e lancio tasker per letture ogni 4 minuti
  Serial.println("End of Setup");
}

void loop()
{
  //irRoutine();

  mqttWifi::gestisciConnessione();
  smartDelay(1000);
}
