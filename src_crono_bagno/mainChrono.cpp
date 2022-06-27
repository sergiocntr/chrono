#include <mainChrono.h>
void blinkLed(uint8_t volte){
  for (uint8_t i = 0; i < volte; i++)
  {
    digitalWrite(LED_BUILTIN,LOW);
    delay(250);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(250);
  }
}
void adessoDormo(){
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
void setupWifi(){
  WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  WiFi.disconnect(true);    // Delete SDK wifi config
  delay(200);
  WiFi.setOutputPower(14);        // 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW
  WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  delay(10);
  WiFi.hostname("bagno");      // DHCP Hostname (useful for finding device for static lease)
  WiFi.mode(WIFI_STA);
  WiFi.forceSleepWake();
  delay(10);
  WiFi.config(ipBagno, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  delay(10);
  WiFi.begin(ssid, password);
}
void setup() {
  setupWifi();
  delay(10);
  wifi_initiate = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - wifi_initiate) > 5000L) {
      adessoDormo();
      //dopo c'e' il restart
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
      //blinkLed(5);
      adessoDormo();
      //dopo c'e' il restart
    }
    delay(500);
  } 
  delay(50);
  reconnect();
 // if(mqttOK){blinkLed(3);}
  DS18B20.begin();
  delay(10);

  nex_routines();
  
  delay(10);
  wifi_initiate=millis();
  getLocalTemp();
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
    NtxtOra.setText(newFWVersion.c_str());
    smartDelay(1000);
    if( newVersion > versione ) {
    //  Serial.println( "Preparing to update" );
      client.disconnect();
      smartDelay(1000);
      t_httpUpdate_return ret = ESPhttpUpdate.update( myLocalConn,fwImageURL );
      yield();
      switch(ret) {
        case HTTP_UPDATE_FAILED:
          NtxtOra.setText("U_F");
          //check=1; //Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          NtxtOra.setText("N_U");
          //check=2;//Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          //Serial.println("[update] Update ok."); // may not called we reboot the ESP
          break;
      }
      smartDelay(1000);
    }
    else {
      NtxtOra.setText("S_V");
    }
  }
  else {
    //Serial.print( "Firmware version check failed, got HTTP response code " );
    //Serial.println( httpCode );
    //check= httpCode;
    NtxtOra.setText("A_E");
  }
  httpClient.end();
  myLocalConn.stop();
  }
void getLocalTemp(){
  float temp=22.22;
  DS18B20.requestTemperatures();
  delay(10);
  temp = DS18B20.getTempCByIndex(0);
  smartDelay(100);
  if(temp == 85.0 || temp == (-127.0)){temp=22.22;}
    //temp*=100;
    temp = roundf(temp * 100) / 100;
  delay(10);
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  delay(10);
  root["topic"]="BagnoChr";
  root["temp"] = temp;
  smartDelay(100);
  char JSONmessageBuffer[256];
  root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  smartDelay(100);
  mqttOK = client.publish(casaSensTopic,JSONmessageBuffer,sizeof(JSONmessageBuffer));
  smartDelay(100);
  NtxtStanza.setValue(temp*10);
  if((RisOn) && (temp > 23 )){
    mqttOK = client.publish(riscaldaTopic, "0");
    db_array_value[1] = 0;
    NcrRis.setPic(0);
    RisOn = false;
  }
}
void reconnect() {
  smartDelay(50);
  client.publish(logTopic, "Bagno connesso");
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
  mqttOK=client.subscribe(updateTopic);
  // delay(10);
  // mqttOK=client.subscribe(eneValTopic);
  smartDelay(50);
}
void loop() {
  const uint32_t wifi_check_time=240000L;
  uint32_t timeNow=millis();

  static uint32_t temp_check= timeNow;
  yield();
  
  if(((timeNow - temp_check) > 30000L) && RisOn) {
    getLocalTemp();
    temp_check= timeNow;
  }
  if((timeNow - wifi_initiate) > wifi_check_time){ 
    wifi_initiate=timeNow;
    if(!mqttOK)
    {
      adessoDormo();
    }
    getLocalTemp();
  }
  smartDelay(1000);

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
  smartDelay(200);
  uint8_t check=0;
  if(strcmp(topic,systemTopic) == 0){
    if (char(payload[0]) == '0') {
      delay(10);
      check=1;
      adessoDormo();
    }
  }
  else if(strcmp(topic,updateTopic) == 0){
    if (char(payload[0]) == '3') {
      delay(10);
      check=1;
      NtxtOra.setText("UPMQ");
      checkForUpdates();
    }
  }
  else if(strcmp(topic, acquaTopic) == 0 ) {
    check=1;
    if (char(payload[0]) == '0') {
      ////DEBUG_PRINT(db_array_value[2]);
      db_array_value[2] = 0;
      //db_array_value[2] = 0;
      NcrAcq.setPic(0);
      //NcrAlert.setPic(0);
      // sendCommand("ref 7");
      // sendCommand("ref 5");
      AlarmOn = false;
      AcqOn = false;
    }else if (char(payload[0]) == '1'){
      db_array_value[2] = 2;
      NcrAcq.setPic(2);
      AcqOn=true;
    }
    else if (char(payload[0]) == '2'){
      db_array_value[2] = 2;
      //NcrAlert.setPic(2);
      AlarmOn = true;
    }
  }
  else if(strcmp(topic, riscaldaTopic) == 0 ) {
    check=1;
    if (char(payload[0]) == '0') {
      db_array_value[1] = 0;
      NcrRis.setPic(0);
      RisOn = false;

    }else{
      db_array_value[1] = 2;
      NcrRis.setPic(2);
      RisOn = true;
    }
  }
  //update_buttons();
  if(check) return;
  smartDelay(200);
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
      NtxtOra.setText(Nex_Time);
      Nday.setText(Nex_Day);
    }
  }
  // else if(strcmp(topic, casaSensTopic) == 0 ) {
  //   //String msg_Topic = root["topic"];
  //   if(msg_Topic == "DHTCamera") {
  //     const char* Nex_inHm = root["Hum"];
  //     const char* Nex_inTemp = root["Temp"];
  //     NtxtOra.setText(Nex_inTemp);
  //     Nin_hum.setText(Nex_inHm);
  //     delay(10);
  //   }
  // }
  else if(strcmp(topic, extSensTopic) == 0 ) {
    //String msg_Topic = root["topic"];
    if(msg_Topic == "Caldaia") {
      const char* Nex_wt = root["acqua"];
      NtxtAcq.setText(Nex_wt);
      int power = atoi(root["power"]);
      (power > 100) ? CaldOn = true : CaldOn = false;
    }
    // else if (msg_Topic == "Terrazza"){
    //   const char* Nex_outHm = root["Hum"];
    //   const char* Nex_outTemp = root["Temp"];
    //   Nout_temp.setText(Nex_outTemp);
    //   Nout_hum.setText(Nex_outHm);
    // }
  }
  // else if(strcmp(topic, eneValTopic) == 0 ) {
  //   //String msg_Topic = root["topic"];
  //   if(msg_Topic == "EneMain") {
  //     const char* Nex_eneVal = root["e"];
  //     Nset_temp.setText(Nex_eneVal);
  //   }
  // }
  smartDelay(200);
  #endif
  }
void smartDelay(unsigned long mytime){
  uint32_t adesso = millis();
  static uint32_t lampAlarm = millis();
  while((millis()-adesso)<mytime   ){
    client.loop();
    if((millis()-lampAlarm) > 300){
      if(AlarmOn ) toggle_alarm();
      if((CaldOn ) && (AcqOn)) toggle_acq();
      lampAlarm = millis();
    } 
    nexLoop(nex_listen_list);
    delay(10);
  }
}
void stampaDebug(int8_t intmess){
  String myMess;
  switch (intmess) {
    case 0:
    myMess="W_OK";
      break;
    case 1:
      myMess="W_KO";

      break;
    case 2:
      myMess="M_OK";

      break;
    case 3:
      myMess="M_KO";

      break;
  }
  NtxtOra.setText(myMess.c_str());
  smartDelay(2000);
}
uint8_t toggle_button(int value){ // qui cambia dalla immagiune 0 alla 2
  if (db_array_value[value] == 2) {
    db_array_value[value] = 0;
    return 0;
  } else {
    db_array_value[value] = 2;
    return 2;
  }
}
void toggle_alarm(){ // ALLARME SI STA PER SPEGNERE L ACQUA LAMPEGGIA PUNTO ESCLAMATIVO
  if (db_array_value[2] != 1) {
    db_array_value[2] = 1;
    
  } else {
    db_array_value[2] = 3;
  }
  NcrAcq.setPic(db_array_value[2]);
}
void toggle_acq(){    // LAMPEGGIA IL ROSSO QUANDO SCALDA
  if (db_array_value[2] != 1) {
    db_array_value[2] = 1;
    
  } else {
    db_array_value[2] = 2;
  }
  NcrAcq.setPic(db_array_value[2]);
}
void update_buttons(){
    NcrRis.setPic(db_array_value[1]);
    NcrAcq.setPic(db_array_value[2]);
    //NcrAlert.setPic(db_array_value[2]);
}
void Nrisc_onPushCallback(void *ptr){
  uint8_t number = toggle_button(1);
  NcrRis.setPic(number);
  if (number == 0) {
    client.publish(riscaldaTopic, "0");
  } else {
    client.publish(riscaldaTopic, "1");
  }
}
void Nwater_onPushCallback(void *ptr){
  uint8_t number = toggle_button(2);
    NcrAcq.setPic(number);
    if (number == 0) {
      mqttOK=client.publish(acquaTopic, "0");
    } else {
      mqttOK=client.publish(acquaTopic, "1");
    }
}
void nex_routines(){
  nexInit();
  delay(10);
  sendCommand("dim=20");
  //Nset_temp.setText("");
  NtxtOra.setText("");
  NtxtAcq.setText("");
  Nday.setText("");
  NcrRis.attachPush(Nrisc_onPushCallback);
  NcrAcq.attachPush(Nwater_onPushCallback);
  //Nb_up.attachPush(Nb_upPushCallback);
  //Nb_down.attachPush(Nb_downPushCallback);
  //update_buttons();
  sendCommand("sleep=0");
  }
