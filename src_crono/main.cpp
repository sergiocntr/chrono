#include <main.h>
// char buffer[15]={0};
//  void pl(uint8_t num){
//    Serial.println(num);
//  }
void blinkLed(uint8_t volte)
{
  for (uint8_t i = 0; i < volte; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
}
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
  delay(1000); // inizializzazione generale
  Serial.begin(9600);
  mqttWifi::randomDelayAtBoot();
  mqttWifi::setupWifi(); // prepara e chiama WiFi.begin
  if (!mqttWifi::connectWifi())
  {                           // attende connessione
    mqttWifi::adessoDormo(1); // entra in sleep se fallisce
  }

  mqttWifi::setupMqtt(); // setServer, setCallback, connect MQTT
  mqttWifi::reconnect(); // publish iniziale + subscribe

  irrecv.enableIRIn();      // IR
  wifi_initiate = millis(); // timestamp

  bool nexTest = nexchr::nex_routines(); // init Nextion
  if (!nexTest){
    mqttWifi::publish(logTopic, "Chrono : Nextion Fail!");
    mqttWifi::adessoDormo(8); // entra in sleep se fallisce

  }

  tempDHT::setupTemp();    // init DHT
  tempDHT::getLocalTemp(); // lettura iniziale
}

void loop()
{
  irRoutine();
  yield();
  if ((millis() - wifi_initiate) > wifi_check_time)
  {
    wifi_initiate = millis();
    if (!mqttWifi::connectWifi())
  {                           // attende connessione
    mqttWifi::adessoDormo(8); // entra in sleep se fallisce
  }
    tempDHT::setupTemp();
    smartDelay(500);
    tempDHT::getLocalTemp();
  }
  smartDelay(1000);
}
