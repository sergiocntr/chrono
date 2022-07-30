#include <main.h>
#include <Int64String.h>
//char buffer[15]={0};
void pl(uint8_t num){
  Serial.println(num);
}
void blinkLed(uint8_t volte){
  for (uint8_t i = 0; i < volte; i++)
  {
    digitalWrite(LED_BUILTIN,LOW);
    delay(250);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(250);
  }
}
void smartDelay(unsigned long mytime){
  uint32_t adesso = millis();
  while((millis()-adesso)<mytime){
    mqttWifi::client.loop();
    nexLoop(nex_listen_list);
    delay(10);
  }
}

void irRoutine(){
  if (irrecv.decode(&results)) {
    uint64_t infraredNewValue = results.value;
    switch (infraredNewValue) {
      case monnezza:
        mqttWifi::publish(teleTopic, "monnezza");
        break;
      case spegni:
       mqttWifi::publish(teleTopic,"spegni");
       delay(10);
       mqttWifi::adessoDormo();
        break;
      case acquaON:
        mqttWifi::publish(acquaTopic,"1");
        break;
      case eneOff:
        mqttWifi::publish(eneTopic,"0");
        break;
      default:
        String s=int64String(infraredNewValue,HEX,false);
        mqttWifi::publish(iRTopic,s.c_str());
        break;
    }
    yield();
    irrecv.resume();  // Receive the next value
  }
}

void setup() {
delay(2000);
Serial.begin(9600);
  delay(10);
  mqttWifi::setupWifi();
  delay(10);
  mqttWifi::setupMqtt();
  delay(10);
  mqttWifi::connectWifi();
  delay(10);
  mqttWifi::reconnect();
  delay(10);
  nexchr::nex_routines();
  delay(10);
  irrecv.enableIRIn();  // Start the receiver
  delay(10);
  wifi_initiate=millis();
  tempDHT::setupTemp();
  tempDHT::getLocalTemp();
}

void loop() {
  irRoutine();
  yield();
  if((millis() - wifi_initiate) > wifi_check_time){ 
    wifi_initiate = millis();
    tempDHT::setupTemp();
    smartDelay(500);
    tempDHT::getLocalTemp();
  }
  smartDelay(1000);
}

/* void sendMySql(char* temp,char* hum){
  //WiFiClient mySqlclient;
  if (mywifi.connect(host, httpPort))
  {
    String s =String("GET /meteofeletto/chrono_logger.php?temp=" + String(temp) +
    +"&&pwd=" + webpass +
    +"&&hum=" + String(hum) +
    + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    smartDelay(100);
    mywifi.println(s);
    smartDelay(100);
    //mywifi.stop();
  }
} */
//boolean screenOff = true;

// void sendData();
// void checkForUpdates();
// void reconnect();
// void checkConn();
// void callback(char* topic, byte* payload, unsigned int length);
// void nex_routines();
// void Nwater_onPushCallback(void *ptr);
// void Nrisc_onPushCallback(void *ptr);
// void Nb_AlarmCallback(void *ptr);
// //void Nb_downPushCallback(void *ptr);
// //void Nb_upPushCallback(void *ptr);
// void update_buttons();
// uint8_t toggle_button(int value);
// void stampaDebug(int8_t intmess);
// void smartDelay(unsigned long mytime);
// void getLocalTemp();
// void irRoutine();