#include "mqttWifiMessages.h"
namespace mqttWifi
{
  // ========== GESTIONE PUBBLICAZIONE ==========
  bool publish(const char *topic, const char *message, bool retained)
  {
    if (!client.connected())
    {
      Serial.println("[PUBLISH] Client non connesso");
      return false;
    }

    // Tentativi multipli con delay ridotto
    for (size_t tentativo = 0; tentativo < 3; tentativo++)
    {
      client.loop(); // Mantieni connessione attiva
      
      // publish(topic, payload, retained)
      if (client.publish(topic, message, retained))
      {
        Serial.printf("[PUBLISH] OK su tentativo %d %s\n", 
                     tentativo + 1, 
                     retained ? "(RETAINED)" : "");
        return true; // Successo
      }
      
      Serial.printf("[PUBLISH] Fallito tentativo %d/3\n", tentativo + 1);
      delay(50); // Breve pausa tra tentativi
    }

    // Dopo 3 tentativi falliti, entra in modalità riposo
    adessoDormo(8, PUBLISH_FALLITO);
    return false;
  }

  // ========== INVIO DATI SENSORI ==========
  void sendData()
  {
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    
    root["topic"] = "SalChr";
    root["hum"] = myTemp.h / 4;
    root["temp"] = myTemp.t / 4;
    root["cf"] = myTemp.confort;
    
    char JSONmessageBuffer[256];
    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    
    Serial.println("[SENDDATA] Invio dati temperatura...");
    
    // Usa retained=true per i dati dei sensori
    // così i nuovi client vedono subito l'ultimo valore
    if (publish(casaSensTopic, JSONmessageBuffer, true))
    {
      // Reset solo se pubblicazione riuscita
      myTemp.h = 0;
      myTemp.t = 0;
      Serial.println("[SENDDATA] ✓ Dati inviati con successo");
    }
    else
    {
      Serial.println("[SENDDATA] ✗ Invio fallito");
    }
  }

  // ========== INVIO COMANDI TENDE ==========
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

    Serial.println("[SENDTENDE] Invio comando tende...");
    
    // Comandi tende: retained=false (sono comandi temporanei)
    if (publish(tendeTuya, JSONmessageBuffer, false))
    {
      Serial.println("[SENDTENDE] ✓ Comando inviato");
    }
    else
    {
      Serial.println("[SENDTENDE] ✗ Invio fallito");
    }
  }

  // ========== AGGIORNAMENTO FIRMWARE ==========
  void checkForUpdates()
  {
    Serial.println("[UPDATE] Controllo aggiornamenti firmware...");
    
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
      
      Serial.printf("[UPDATE] Versione corrente: %d, Versione disponibile: %d\n", 
                   versione, newVersion);
      
      Ntcurr.setText(newFWVersion.c_str());
      delay(1000);

      if (newVersion > versione)
      {
        Serial.println("[UPDATE] Nuova versione disponibile! Avvio update...");
        
        client.disconnect();
        delay(1000);
        
        t_httpUpdate_return ret = httpUpdate.update(myLocalConn, fwImageURL);
        
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
          Serial.println("[UPDATE] ✗ Aggiornamento FALLITO");
          Ntcurr.setText("U_F");
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("[UPDATE] Nessun aggiornamento disponibile");
          Ntcurr.setText("N_U");
          break;
        case HTTP_UPDATE_OK:
          Serial.println("[UPDATE] ✓ Aggiornamento completato - Riavvio...");
          // Riavvio automatico
          break;
        }
      }
      else
      {
        Serial.println("[UPDATE] Firmware già aggiornato");
        Ntcurr.setText("S_V");
      }
    }
    else
    {
      Serial.printf("[UPDATE] ✗ Errore HTTP: %d\n", httpCode);
      Ntcurr.setText("A_E");
    }
    
    httpClient.end();
    myLocalConn.stop();
  }

  // ========== CALLBACK MQTT ==========
  void callback(char *topic, byte *payload, unsigned int length)
  {
#ifdef DEBUGMIO
    Serial.print("[CALLBACK] Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    Serial.println();
#else
    Serial.printf("[CALLBACK] Ricevuto da topic: %s\n", topic);
    
    delay(20);
    bool handled = false;

    // ========== GESTIONE COMANDI DI SISTEMA ==========
    if (strcmp(topic, systemTopic) == 0)
    {
      if (char(payload[0]) == '0')
      {
        handled = true;
        Serial.println("[CALLBACK] Comando SLEEP ricevuto");
        adessoDormo(8, COMANDO_SYSTEM_TOPIC);
        return; // Esci immediatamente dopo dormire
      }
    }
    else if (strcmp(topic, updateTopic) == 0)
    {
      if (char(payload[0]) == '0')
      {
        handled = true;
        Serial.println("[CALLBACK] Comando UPDATE ricevuto");
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
      Serial.printf("[CALLBACK] Stato acqua aggiornato: %d\n", db_array_value[2]);
    }
    else if (strcmp(topic, riscaldaTopic) == 0)
    {
      handled = true;
      db_array_value[1] = (char(payload[0]) == '0') ? 0 : 1;
      Nrisc_on.setPic(db_array_value[1]);
      Serial.printf("[CALLBACK] Stato riscaldamento aggiornato: %d\n", db_array_value[1]);
    }

    if (handled)
    {
      return;
    }

    // ========== PARSING JSON PER MESSAGGI COMPLESSI ==========
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(payload);
    
    if (!root.success())
    {
      Serial.println("[CALLBACK] JSON non valido, ignoro messaggio");
      return;
    }

    String msg_Topic = root["topic"];
    Serial.printf("[CALLBACK] Topic JSON interno: %s\n", msg_Topic.c_str());

    // ========== GESTIONE MESSAGGI PER TOPIC ==========
    if (strcmp(topic, systemTopic) == 0)
    {
      if (msg_Topic == "UpTime")
      {
        const char *Nex_Time = root["hours"];
        const char *Nex_Day = root["Day"];
        Ncurr_hour.setText(Nex_Time);
        Nday.setText(Nex_Day);
        Serial.printf("[CALLBACK] Aggiornato orario: %s, giorno: %s\n", Nex_Time, Nex_Day);
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
        Serial.printf("[CALLBACK] DHT Camera - Temp: %s°C, Hum: %s%%\n", Nex_inTemp, Nex_inHm);
      }
    }
    else if (strcmp(topic, extSensTopic) == 0)
    {
      if (msg_Topic == "Caldaia")
      {
        const char *Nex_wt = root["acqua"];
        Ncurr_water_temp.setText(Nex_wt);
        Serial.printf("[CALLBACK] Temp acqua caldaia: %s°C\n", Nex_wt);
      }
      else if (msg_Topic == "Terrazza")
      {
        const char *Nex_outHm = root["Hum"];
        const char *Nex_outTemp = root["Temp"];
        Nout_temp.setText(Nex_outTemp);
        Nout_hum.setText(Nex_outHm);
        Serial.printf("[CALLBACK] Terrazza - Temp: %s°C, Hum: %s%%\n", Nex_outTemp, Nex_outHm);
      }
    }
    else if (strcmp(topic, eneValTopic) == 0)
    {
      if (msg_Topic == "EneMain")
      {
        const char *Nex_eneVal = root["e"];
        Nset_temp.setText(Nex_eneVal);
        Serial.printf("[CALLBACK] Valore energia: %s\n", Nex_eneVal);
      }
    }

    delay(50);
    if (client.connected())
    {
      client.loop();
    }
#endif
  }

} // namespace mqttWifi