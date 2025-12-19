#pragma once
#include "myIP.h"
#include "password.h"
#include "topic.h"
WiFiClient mywifi;
WiFiClient c;

namespace mqttWifi
{
  PubSubClient client(c);

  bool mqttOK = 0;
  void adessoDormo(uint8_t mode);
  void publish(const char *topic, const char *message)
  {
    for (size_t i = 0; i < 4; i++)
    {
      delay(10);
      client.loop();
      mqttOK = client.publish(topic, message, strlen(message));
      if(mqttOK) continue;
      
    }
     
    if (!mqttOK)
      adessoDormo(8);
  }
  void adessoDormo(uint8_t mode) // 0 = non comandi a nextion // 1 no comandi wifi // 8  =tutto
  {
    if (mode > 0)
    {
      sendCommand("thup=1");
      sendCommand("sleep=1");
    }
    if (mode > 1)
    {
      if (client.connected())
      {
        client.disconnect(); // Chiude connessione TCP pulitamente
        client.loop();
        delay(100);
      }
      if (WiFi.status() == WL_CONNECTED)
      {
        WiFi.disconnect(); // SENZA true, non serve cancellare credenziali
        delay(100);        // Tempo per inviare il pacchetto
      }
    }
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
    WiFi.mode(WIFI_OFF); // energy saving mode if local WIFI isn't connected
    
    WiFi.forceSleepBegin();
    delay(300000);
    ESP.reset();
  }

  void sendData()
  {

    delay(10);
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    delay(10);
    root["topic"] = "SalChr";
    root["hum"] = myTemp.h / 4;
    root["temp"] = myTemp.t / 4;
    // root["cnf"] = cnf;
    delay(10);
    char JSONmessageBuffer[256];
    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    publish(casaSensTopic, JSONmessageBuffer);
    myTemp.h = 0;
    myTemp.t = 0;
    delay(10);
  }

  void sendTende(Tende tendeTargets[], size_t numTende, ComandoTende comando, int percentuale)
  {
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    // Usa una variabile temporanea per l'enum comando
    int comandoInt = (int)comando;
    root["comando"] = comandoInt; // Assegna la versione int

    if (comando == 2)
    { // APRI PARZIALE
      root["percentuale"] = percentuale;
    }

    JsonArray &tendeArray = root.createNestedArray("tende");
    for (size_t i = 0; i < numTende; i++)
    {
      tendeArray.add((int)tendeTargets[i]); // Conversione esplicita a int
    }

    delay(10);
    char JSONmessageBuffer[256];
    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

    publish(tendeTuya, JSONmessageBuffer);
  }

  void checkForUpdates()
  {

    String fwURL = String(fwUrlBase);
    fwURL.concat(mqttId);
    yield();
    String fwVersionURL = fwURL;
    fwVersionURL.concat("/version.php");
    // Serial.print( "Firmware version URL: " );
    // Serial.println( fwVersionURL );
    yield();
    String fwImageURL = fwURL;
    fwImageURL.concat("/firmware.bin");
    // Serial.print( "Firmware  URL: " );
    // Serial.prconst char* mqttID;intln( fwImageURL );
    yield();
    WiFiClient myLocalConn;
    HTTPClient httpClient;
    httpClient.begin(myLocalConn, fwVersionURL);
    int httpCode = httpClient.GET();
    if (httpCode == 200)
    {
      String newFWVersion = httpClient.getString();
      int newVersion = newFWVersion.toInt();
      Ntcurr.setText(newFWVersion.c_str());
      delay(1000);
      if (newVersion > versione)
      {
        //  Serial.println( "Preparing to update" );
        client.disconnect();
        delay(1000);
        t_httpUpdate_return ret = ESPhttpUpdate.update(myLocalConn, fwImageURL);
        yield();
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
          Ntcurr.setText("U_F");
          // thisUpdate = UP_FAIL;
          // check=1; //Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Ntcurr.setText("N_U");
          // thisUpdate = NO_UP;
          // check=2;//Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          // Serial.println("[update] Update ok."); // may not called we reboot the ESP

          break;
        }
        delay(100);
      }
      else
      {
        Ntcurr.setText("S_V");
        // thisUpdate = SAME_VER;
      }
    }
    else
    {
      Ntcurr.setText("A_E");
      // thisUpdate = CONN_FAIL;
    }
    httpClient.end();
    myLocalConn.stop();
  }
  // Funzione per convertire JSON e ricomporre con valori in const char*
  /* void convertJsonToString( StaticJsonDocument& input, StaticJsonDocument& output) {
      for (JsonPair kv : input) {
          const char* key = kv.key().c_str();

          if (kv.value().is<const char*>()) {
              // Se è una stringa, copialo direttamente
              output[key] = kv.value().as<const char*>();
          } else if (kv.value().is<double>()) {
              // Se è un double, convertilo in stringa
              double num = kv.value().as<double>();
              char buffer[20];
              dtostrf(num, 6, 2, buffer); // Converte double con 2 cifre decimali
              output[key] = buffer; // Imposta il valore convertito
          }
      }
  } */
  void callback(char *topic, byte *payload, unsigned int length)
  {
#ifdef DEBUGMIO
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    Serial.println();
#else
    delay(20);
    uint8_t check = 0;
    if (strcmp(topic, systemTopic) == 0)
    {
      if (char(payload[0]) == '0')
      {
        delay(10);
        check = 1;
        adessoDormo(8);
      }
    }
    else if (strcmp(topic, updateTopic) == 0)
    {
      if (char(payload[0]) == '0')
      {
        delay(10);
        check = 1;
        Ntcurr.setText("UPMQ");
        checkForUpdates();
      }
    }
    else if (strcmp(topic, acquaTopic) == 0)
    {
      check = 1;
      if (char(payload[0]) == '0')
      {
        ////DEBUG_PRINT(db_array_value[2]);
        db_array_value[2] = 0;
        Nwater_on.setPic(0);
      }
      else
      {
        db_array_value[2] = 1;
        Nwater_on.setPic(1);
      }
    }
    else if (strcmp(topic, riscaldaTopic) == 0)
    {
      check = 1;
      if (char(payload[0]) == '0')
      {
        db_array_value[1] = 0;
        Nrisc_on.setPic(0);
      }
      else
      {
        db_array_value[1] = 1;
        Nrisc_on.setPic(1);
      }
    }
    if (check)
      return;
    delay(10);
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(payload);
    String msg_Topic = root["topic"];
    if (strcmp(topic, systemTopic) == 0)
    {
      // String msg_Topic = root["topic"];
      if (msg_Topic == "UpTime")
      {
        // wifi_initiate=millis();
        delay(10);
        const char *Nex_Time = root["hours"];
        const char *Nex_Day = root["Day"];
        Ncurr_hour.setText(Nex_Time);
        Nday.setText(Nex_Day);
      }
    }
    else if (strcmp(topic, casaSensTopic) == 0)
    {
      // String msg_Topic = root["topic"];
      if (msg_Topic == "DHTCamera")
      {
        const char *Nex_inHm = root["Hum"];
        const char *Nex_inTemp = root["Temp"];
        Ntcurr.setText(Nex_inTemp);
        Nin_hum.setText(Nex_inHm);
        delay(10);
      }
    }
    else if (strcmp(topic, extSensTopic) == 0)
    {
      // String msg_Topic = root["topic"];
      if (msg_Topic == "Caldaia")
      {
        const char *Nex_wt = root["acqua"];
        Ncurr_water_temp.setText(Nex_wt);
      }
      else if (msg_Topic == "Terrazza")
      {
        const char *Nex_outHm = root["Hum"];
        const char *Nex_outTemp = root["Temp"];
        Nout_temp.setText(Nex_outTemp);
        Nout_hum.setText(Nex_outHm);
      }
    }
    else if (strcmp(topic, eneValTopic) == 0)
    {
      // String msg_Topic = root["topic"];
      if (msg_Topic == "EneMain")
      {
        const char *Nex_eneVal = root["e"];
        Nset_temp.setText(Nex_eneVal);
      }
    }
    delay(100);
    client.loop();
#endif
  }

  void setupWifi()
  {
    WiFi.persistent(false);
    WiFi.disconnect(true); // reset SDK config
    delay(200);

    WiFi.setOutputPower(14); // riduce assorbimento
    WiFi.mode(WIFI_OFF);     // spegne per forzare stato pulito
    delay(10);
    WiFi.hostname("chrono");
    WiFi.mode(WIFI_STA);
    WiFi.forceSleepWake(); // workaround per bug init
    delay(10);

    WiFi.config(ipChrono, gateway, subnet, dns1); // static IP
    delay(10);
    WiFi.begin(ssid, password); // connessione
     delay(100);
  }

  void setupMqtt()
  {
    String clientId = String(mqttId) + String(random(0xffff), HEX);
    delay(10);

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    delay(100);

    uint32_t start = millis();
    while (!client.connected())
    {
      if (millis() - start > 5000)
      {
        mqttOK = 1; // flag di errore
        return;
      }
      client.connect(clientId.c_str(), mqttUser, mqttPass);
      delay(250);
    }

    mqttOK = 0; // ok
    delay(50);
  }
  void randomDelayAtBoot()
  {

    // Genera un ritardo unico per ogni ESP basato sulla sua MAC
    uint8_t mac[6];
    WiFi.macAddress(mac);

    // Usa gli ultimi 2 byte del MAC per creare un ritardo da 0 a 10 secondi
    unsigned long delayMs = ((mac[4] + mac[5]) % 100) * 100; // 0-9900ms

    // Serial.printf("Ritardo connessione: %lu ms (MAC: %02X:%02X)\n",
    //               delayMs, mac[4], mac[5]);

    delay(delayMs);
  }

  bool connectWifi()
  {
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
      if (millis() - start > 7000)
        return false; // timeout

      delay(250);
    }

    return true;
  }
  bool reconnect()
  {
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
    mqttOK = client.subscribe(eneValTopic);
    client.loop(); // importante per completare handshake
    return mqttOK;
  }
}