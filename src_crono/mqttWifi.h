#pragma once
#include <impostazioni.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "myIP.h"
#include "password.h"
#include "topic.h"
WiFiClient mywifi;
WiFiClient c;

namespace mqttWifi{
  PubSubClient client(c);

bool  mqttOK=0;
void  publish(const char* topic,const char* message ){
  mqttOK = client.publish(topic, message,sizeof(message));
  
}

void sendData(){
    //byte cnf = dht.computePerception(tH.temperature , tH.humidity);
    delay(10);
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    delay(10);
    root["topic"]="SalChr";
    root["hum"] = myTemp.h/4;
    root["temp"] = myTemp.t/4;
    //root["cnf"] = cnf;
    delay(10);
    char JSONmessageBuffer[256];
    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    publish(casaSensTopic,JSONmessageBuffer);
    myTemp.h = 0;
    myTemp.t = 0;
    delay(10);

}
// void stampaDebug(int8_t intmess){
//   String myMess;
//   switch (intmess) {
//     case 0:
//     myMess="W_OK";
//       break;
//     case 1:
//       myMess="W_KO";
//       break;
//     case 2:
//       myMess="M_OK";
//       break;
//     case 3:
//       myMess="M_KO";
//       break;
//   }
//   Ntcurr.setText(myMess.c_str());
//   smartDelay(2000);
// }
void adessoDormo(){
  mqttOK=1;
  sendCommand("thup=1");
  sendCommand("sleep=1");
  client.disconnect();
  delay(10);
  WiFi.disconnect(true);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  WiFi.forceSleepBegin();
  delay(180000);
  ESP.reset();
}
void checkForUpdates() {
  
  String fwURL = String( fwUrlBase );
  fwURL.concat( mqttId );
  yield();
  String fwVersionURL = fwURL;
  fwVersionURL.concat( "/version.php" );
  //Serial.print( "Firmware version URL: " );
  //Serial.println( fwVersionURL );
  yield();
  String fwImageURL = fwURL;
  fwImageURL.concat( "/firmware.bin" );
  //Serial.print( "Firmware  URL: " );
  //Serial.prconst char* mqttID;intln( fwImageURL );
  yield();
  WiFiClient myLocalConn;
  HTTPClient httpClient;
  httpClient.begin( myLocalConn,fwVersionURL );
  int httpCode = httpClient.GET();
  if( httpCode == 200 ) {
    String newFWVersion = httpClient.getString();
    int newVersion = newFWVersion.toInt();
    Ntcurr.setText(newFWVersion.c_str());
    delay(1000);
    if( newVersion > versione ) {
    //  Serial.println( "Preparing to update" );
      client.disconnect();
      delay(1000);
      t_httpUpdate_return ret = ESPhttpUpdate.update( myLocalConn,fwImageURL );
      yield();
      switch(ret) {
        case HTTP_UPDATE_FAILED:
          Ntcurr.setText("U_F");
          //thisUpdate = UP_FAIL;
          //check=1; //Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Ntcurr.setText("N_U");
          //thisUpdate = NO_UP;
          //check=2;//Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          //Serial.println("[update] Update ok."); // may not called we reboot the ESP
          
          break;
      }
      delay(100);
    }
    else {
      Ntcurr.setText("S_V");
      //thisUpdate = SAME_VER;
    }
  }
  else {
    Ntcurr.setText("A_E");
    //thisUpdate = CONN_FAIL;
  }
  httpClient.end();
  myLocalConn.stop();
  }
void callback(char* topic, byte* payload, unsigned int length){
  #ifdef DEBUGMIO
  Serial.print("Message arrived [");  
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  #else
  delay(20);
  uint8_t check=0;
  if(strcmp(topic,systemTopic) == 0){
    if (char(payload[0]) == '0') {
      delay(10);
      check=1;
      adessoDormo();
    }
  }
  else if(strcmp(topic,updateTopic) == 0){
    if (char(payload[0]) == '0') {
      delay(10);
      check=1;
      Ntcurr.setText("UPMQ");
      checkForUpdates();
    }
  }
  else if(strcmp(topic, acquaTopic) == 0 ) {
    check=1;
    if (char(payload[0]) == '0') {
      ////DEBUG_PRINT(db_array_value[2]);
      db_array_value[2] = 0;
      Nwater_on.setPic(0);
    }else{
      db_array_value[2] = 1;
      Nwater_on.setPic(1);
    }
  }
  else if(strcmp(topic, riscaldaTopic) == 0 ) {
    check=1;
    if (char(payload[0]) == '0') {
      db_array_value[1] = 0;
      Nrisc_on.setPic(0);
    }else{
      db_array_value[1] = 1;
      Nrisc_on.setPic(1);
    }
  }
  if(check) return;
  delay(10);
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  String msg_Topic = root["topic"];
  if(strcmp(topic, systemTopic) == 0 ) {
    //String msg_Topic = root["topic"];
    if(msg_Topic == "UpTime") {
      //wifi_initiate=millis();
      delay(10);
      const char* Nex_Time = root["hours"];
      const char* Nex_Day = root["Day"];
      Ncurr_hour.setText(Nex_Time);
      Nday.setText(Nex_Day);
    }
  }
  else if(strcmp(topic, casaSensTopic) == 0 ) {
    //String msg_Topic = root["topic"];
    if(msg_Topic == "DHTCamera") {
      const char* Nex_inHm = root["Hum"];
      const char* Nex_inTemp = root["Temp"];
      Ntcurr.setText(Nex_inTemp);
      Nin_hum.setText(Nex_inHm);
      delay(10);
    }
  }
  else if(strcmp(topic, extSensTopic) == 0 ) {
    //String msg_Topic = root["topic"];
    if(msg_Topic == "Caldaia") {
      const char* Nex_wt = root["acqua"];
      Ncurr_water_temp.setText(Nex_wt);
    }
    else if (msg_Topic == "Terrazza"){
      const char* Nex_outHm = root["Hum"];
      const char* Nex_outTemp = root["Temp"];
      Nout_temp.setText(Nex_outTemp);
      Nout_hum.setText(Nex_outHm);
    }
  }
  else if(strcmp(topic, eneValTopic) == 0 ) {
    //String msg_Topic = root["topic"];
    if(msg_Topic == "EneMain") {
      const char* Nex_eneVal = root["e"];
      Nset_temp.setText(Nex_eneVal);
    }
  }
  delay(100);
  client.loop();
  #endif
  }

void setupWifi(){
  WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  WiFi.disconnect(true);    // Delete SDK wifi config
  delay(200);
  WiFi.setOutputPower(14);        // 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW
  WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  delay(10);
  WiFi.hostname("chrono");      // DHCP Hostname (useful for finding device for static lease)
  WiFi.mode(WIFI_STA);
  WiFi.forceSleepWake();
  delay(10);
  WiFi.config(ipChrono, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  delay(10);
  wl_status_t res =  WiFi.begin(ssid, password);
  //Serial.println(String(res));
}
// void smartDelay(uint32_t tempo){
//   uint32_t adesso = millis();
// }
void setupMqtt(){
String clientId = String(mqttId);
  clientId += String(random(0xffff), HEX);
  delay(10);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(100);
  client.connect(clientId.c_str(),mqttUser,mqttPass);
  delay(10);
  uint32_t wifi_initiate = millis();
  while (!client.connected()) {
    if ((millis() - wifi_initiate) > 5000L) {
      //adessoDormo();
      //dopo c'e' il restart
      mqttOK = 1;
      return;
    }
    //smartDelay(500);
    client.loop();
    delay(100);
  } 
  delay(50);
  mqttOK = 0;
}
bool connectWifi(){
  uint32_t wifi_initiate = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - wifi_initiate) > 5000L) {
      return 1;
      
    }
    delay(500);
  }
  delay(10);
  //blinkLed(2);
  //sendCrash();
  String clientId = String(mqttId);
  clientId += String(random(0xffff), HEX);
  delay(10);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(100);
  client.connect(clientId.c_str(),mqttUser,mqttPass);
  delay(10);
  wifi_initiate = millis();
  while (!client.connected()) {
    if ((millis() - wifi_initiate) > 5000L) {
      return 2;
      //blinkLed(5);
      //adessoDormo();
      //dopo c'e' il restart
    }
    delay(500);
  } 
  delay(50);
  //reconnect();
  return 0;
}
void reconnect() {
  delay(10);
  client.publish(logTopic, "Crono connesso");
  delay(10);
  client.subscribe(systemTopic);
  delay(10);
  client.subscribe(casaSensTopic);
  delay(10);
  client.subscribe(extSensTopic);
  delay(10);
  client.subscribe(acquaTopic);
  delay(10);
  client.subscribe(riscaldaTopic);
  delay(10);
  client.subscribe(updateTopic);
  delay(10);
  mqttOK=client.subscribe(eneValTopic);
  client.loop();
  }
}