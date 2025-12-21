#include <main.h>
void smartDelay(unsigned long mytime)
{
  uint32_t adesso = millis();
  while ((millis() - adesso) < mytime)
  {
    mqttWifi::client.loop();
    nexLoop(nex_listen_list);
    delay(10);
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
      mqttWifi::adessoDormo(8);
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
  //delay(2000); // inizializzazione generale
  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, HIGH);
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
    Serial.println("Nextion Fail");
    mqttWifi::publish(logTopic, "Chrono : Nextion Fail!");
    mqttWifi::adessoDormo(8); // entra in sleep se fallisce
  }
  Serial.println("Nextion OK");
  tempDHT::setupTemp(); // setup DHT22 e lancio tasker per letture ogni 4 minuti
  Serial.println("End of Setup");
}

void loop()
{
  irRoutine();

  mqttWifi::gestisciConnessione();
  smartDelay(1000);
}
