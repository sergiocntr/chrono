#pragma once
#include "myIP.h"
#include "password.h"
#include "topic.h"

WiFiClient mywifi;
WiFiClient c;

namespace mqttWifi
{
  PubSubClient client(c);

  // Stati della connessione
  bool mqttConnesso = false;
  bool inFaseRiposo = false;
  unsigned long timerSenzaConnessione = 0;
  const unsigned long INTERVALLO_RIPROVA = 300000; // 5 minuti
  const unsigned long TIMEOUT_WIFI = 7000;
  const unsigned long TIMEOUT_MQTT = 5000;

  // Forward declarations
  void adessoDormo(uint8_t mode);
  bool reconnectMqtt();

  // ========== GESTIONE PUBBLICAZIONE ==========
  bool publish(const char *topic, const char *message)
  {
    if (!client.connected())
    {
      return false;
    }

    // Tentativi multipli con delay ridotto
    for (size_t tentativo = 0; tentativo < 3; tentativo++)
    {
      client.loop(); // Mantieni connessione attiva
      
      if (client.publish(topic, message, strlen(message)))
      {
        return true; // Successo
      }
      
      delay(50); // Breve pausa tra tentativi
    }

    // Dopo 3 tentativi falliti, entra in modalità riposo
    adessoDormo(8);
    return false;
  }

  // ========== GESTIONE MODALITÀ RIPOSO ==========
  void adessoDormo(uint8_t mode)
  {
    // Spegnimento Nextion (se mode > 0)
    if (mode > 0)
    {
      sendCommand("thup=1");
      sendCommand("sleep=1");
    }

    // Disconnessione pulita
    if (client.connected())
    {
      client.disconnect();
    }
    
    WiFi.disconnect(true); // true = cancella credenziali salvate
    WiFi.mode(WIFI_OFF);

    // Imposta stato di riposo
    inFaseRiposo = true;
    mqttConnesso = false;
    timerSenzaConnessione = millis();
  }

  // ========== INVIO DATI ==========
  void sendData()
  {
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    
    root["topic"] = "SalChr";
    root["hum"] = myTemp.h / 4;
    root["temp"] = myTemp.t / 4;
    
    char JSONmessageBuffer[256];
    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    
    if (publish(casaSensTopic, JSONmessageBuffer))
    {
      // Reset solo se pubblicazione riuscita
      myTemp.h = 0;
      myTemp.t = 0;
    }
  }

  void sendTende(Tende tendeTargets[], size_t numTende, ComandoTende comando, int percentuale)
  {
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root["comando"] = (int)comando;

    if (comando == 2) // APRI PARZIALE
    {
      root["percentuale"] = percentuale;
    }

    JsonArray &tendeArray = root.createNestedArray("tende");
    for (size_t i = 0; i < numTende; i++)
    {
      tendeArray.add((int)tendeTargets[i]);
    }

    char JSONmessageBuffer[256];
    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

    publish(tendeTuya, JSONmessageBuffer);
  }

  // ========== AGGIORNAMENTO FIRMWARE ==========
  void checkForUpdates()
  {
    String fwURL = String(fwUrlBase);
    fwURL.concat(mqttId);
    String fwVersionURL = fwURL + "/version.php";
    String fwImageURL = fwURL + "/firmware.bin";

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
        client.disconnect();
        delay(1000);
        
        t_httpUpdate_return ret = httpUpdate.update(myLocalConn, fwImageURL);
        
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
          Ntcurr.setText("U_F");
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Ntcurr.setText("N_U");
          break;
        case HTTP_UPDATE_OK:
          // Riavvio automatico
          break;
        }
      }
      else
      {
        Ntcurr.setText("S_V");
      }
    }
    else
    {
      Ntcurr.setText("A_E");
    }
    
    httpClient.end();
    myLocalConn.stop();
  }

  // ========== CALLBACK MQTT ==========
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
    bool handled = false;

    // Gestione comandi di sistema
    if (strcmp(topic, systemTopic) == 0)
    {
      if (char(payload[0]) == '0')
      {
        handled = true;
        adessoDormo(8);
        return; // Esci immediatamente dopo dormire
      }
    }
    else if (strcmp(topic, updateTopic) == 0)
    {
      if (char(payload[0]) == '0')
      {
        handled = true;
        Ntcurr.setText("UPMQ");
        checkForUpdates();
        return;
      }
    }
    else if (strcmp(topic, acquaTopic) == 0)
    {
      handled = true;
      db_array_value[2] = (char(payload[0]) == '0') ? 0 : 1;
      Nwater_on.setPic(db_array_value[2]);
    }
    else if (strcmp(topic, riscaldaTopic) == 0)
    {
      handled = true;
      db_array_value[1] = (char(payload[0]) == '0') ? 0 : 1;
      Nrisc_on.setPic(db_array_value[1]);
    }

    if (handled)
    {
      return;
    }

    // Parsing JSON per messaggi complessi
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(payload);
    
    if (!root.success())
    {
      return; // JSON non valido
    }

    String msg_Topic = root["topic"];

    if (strcmp(topic, systemTopic) == 0)
    {
      if (msg_Topic == "UpTime")
      {
        const char *Nex_Time = root["hours"];
        const char *Nex_Day = root["Day"];
        Ncurr_hour.setText(Nex_Time);
        Nday.setText(Nex_Day);
      }
    }
    else if (strcmp(topic, casaSensTopic) == 0)
    {
      if (msg_Topic == "DHTCamera")
      {
        const char *Nex_inHm = root["Hum"];
        const char *Nex_inTemp = root["Temp"];
        Ntcurr.setText(Nex_inTemp);
        Nin_hum.setText(Nex_inHm);
      }
    }
    else if (strcmp(topic, extSensTopic) == 0)
    {
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
      if (msg_Topic == "EneMain")
      {
        const char *Nex_eneVal = root["e"];
        Nset_temp.setText(Nex_eneVal);
      }
    }

    delay(50);
    if (client.connected())
    {
      client.loop();
    }
#endif
  }

  // ========== SETUP INIZIALE ==========
  void setupWifi()
  {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("chrono");
    WiFi.config(ipChrono, gateway, subnet, dns1);
    WiFi.setSleep(true); // Modem sleep per risparmio energetico
  }

  void randomDelayAtBoot()
  {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    unsigned long delayMs = ((mac[4] + mac[5]) % 100) * 100; // 0-9900ms
    delay(delayMs);
  }

  // ========== CONNESSIONE WIFI ==========
  bool connectWifi()
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }

    WiFi.begin(ssid, password);
    uint32_t start = millis();
    
    while (WiFi.status() != WL_CONNECTED)
    {
      if (millis() - start > TIMEOUT_WIFI)
      {
        return false;
      }
      delay(250);
    }

    return true;
  }

  // ========== CONNESSIONE MQTT ==========
  bool connectMqtt()
  {
    if (client.connected())
    {
      return true;
    }

    String clientId = String(mqttId) + String(random(0xffff), HEX);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    uint32_t start = millis();
    while (!client.connected())
    {
      if (millis() - start > TIMEOUT_MQTT)
      {
        return false;
      }

      if (client.connect(clientId.c_str(), mqttUser, mqttPass))
      {
        return reconnectMqtt(); // Sottoscrivi ai topic
      }
      
      delay(250);
    }

    return false;
  }

  // ========== SOTTOSCRIZIONE TOPIC ==========
  bool reconnectMqtt()
  {
    if (!client.connected())
    {
      return false;
    }

    client.publish(logTopic, "Crono connesso");
    delay(10);

    bool success = true;
    success &= client.subscribe(systemTopic);
    delay(10);
    success &= client.subscribe(casaSensTopic);
    delay(10);
    success &= client.subscribe(extSensTopic);
    delay(10);
    success &= client.subscribe(acquaTopic);
    delay(10);
    success &= client.subscribe(riscaldaTopic);
    delay(10);
    success &= client.subscribe(updateTopic);
    delay(10);
    success &= client.subscribe(eneValTopic);
    
    client.loop(); // Completa handshake
    
    mqttConnesso = success;
    return success;
  }

  // ========== RISVEGLIO DOPO RIPOSO ==========
  void risveglioE_Riconnetti()
  {
    WiFi.mode(WIFI_STA); // Riabilita WiFi
    
    if (connectWifi())
    {
      inFaseRiposo = false;
      
      // Riconnetti MQTT
      if (connectMqtt())
      {
        mqttConnesso = true;
        
        // Riaccendi Nextion
        sendCommand("sleep=0");
        sendCommand("thup=0");
      }
      else
      {
        // MQTT fallito, torna a dormire
        adessoDormo(0); // 0 = non rispegnere Nextion
      }
    }
    else
    {
      // WiFi fallito, torna a dormire
      adessoDormo(0);
    }
  }

  // ========== GESTIONE PRINCIPALE (DA CHIAMARE NEL LOOP) ==========
  void gestisciConnessione()
  {
    // Se siamo in modalità riposo
    if (inFaseRiposo)
    {
      // Controlla se è ora di riprovare
      if (millis() - timerSenzaConnessione >= INTERVALLO_RIPROVA)
      {
        risveglioE_Riconnetti();
      }
      return;
    }

    // Verifica WiFi
    if (WiFi.status() != WL_CONNECTED)
    {
      if (!connectWifi())
      {
        adessoDormo(8);
        return;
      }
    }

    // Verifica MQTT
    if (!client.connected())
    {
      if (!connectMqtt())
      {
        adessoDormo(8);
        return;
      }
    }

    // Tutto connesso: mantieni viva la connessione
    client.loop();
    mqttConnesso = true;
  }

  // ========== SETUP COMPLETO (DA CHIAMARE IN setup()) ==========
  void setupCompleto()
  {
    setupWifi();
    randomDelayAtBoot(); // Evita collisioni all'avvio
    
    if (connectWifi())
    {
      connectMqtt();
    }
    else
    {
      adessoDormo(8);
    }
  }

} // namespace mqttWifi