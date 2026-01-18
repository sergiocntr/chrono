#include "mqttWifiMessages.h"
namespace mqttWifi
{
  // ========== GESTIONE PUBBLICAZIONE ==========
  bool publish(const char *topic, const char *message, bool retained)
  {
    if (!client.connected())
    {
      logSerialPrintln("[PUBLISH] Client non connesso");
      return false;
    }

    // Tentativi multipli con delay ridotto
    for (size_t tentativo = 0; tentativo < 3; tentativo++)
    {
      client.loop(); // Mantieni connessione attiva
      
      // publish(topic, payload, retained)
      if (client.publish(topic, message, retained))
      {
        logSerialPrintf("[PUBLISH] OK su tentativo %d %s\n", 
                     tentativo + 1, 
                     retained ? "(RETAINED)" : "");
        return true; // Successo
      }
      
      logSerialPrintf("[PUBLISH] Fallito tentativo %d/3\n", tentativo + 1);
      delay(50); // Breve pausa tra tentativi
    }

    // Dopo 3 tentativi falliti, entra in modalità riposo
    adessoDormo(8, PUBLISH_FALLITO);
    return false;
  }

  // ========== INVIO DATI SENSORI ==========
  void sendData()
  {
    delay(10);
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    
    root["topic"] = "SalChr";
    root["hum"] = myTemp.h / 4;
    root["temp"] = myTemp.t / 4;
    root["cf"] = myTemp.confort;
    
    char JSONmessageBuffer[256];
    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    
    logSerialPrintln("[SENDDATA] Invio dati temperatura...");
    
    // Usa retained=true per i dati dei sensori
    // così i nuovi client vedono subito l'ultimo valore
    if (publish(casaSensTopic, JSONmessageBuffer, true))
    {
      
      logSerialPrintln("[SENDDATA] ✓ Dati inviati con successo");
    }
    else
    {
      logSerialPrintln("[SENDDATA] ✗ Invio fallito");
    }
    delay(10);
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

    logSerialPrintln("[SENDTENDE] Invio comando tende...");
    
    // Comandi tende: retained=false (sono comandi temporanei)
    if (publish(tendeTuya, JSONmessageBuffer, false))
    {
      logSerialPrintln("[SENDTENDE] ✓ Comando inviato");
    }
    else
    {
      logSerialPrintln("[SENDTENDE] ✗ Invio fallito");
    }
  }

  // ========== AGGIORNAMENTO FIRMWARE ==========
  void checkForUpdates()
  {
    logSerialPrintln("[UPDATE] Controllo aggiornamenti firmware...");
    
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
      
      logSerialPrintf("[UPDATE] Versione corrente: %d, Versione disponibile: %d\n", 
                   versione, newVersion);
      
      Ntcurr.setText(newFWVersion.c_str());
      delay(1000);

      if (newVersion > versione)
      {
        logSerialPrintln("[UPDATE] Nuova versione disponibile! Avvio update...");
        
        client.disconnect();
        delay(1000);
        
        t_httpUpdate_return ret = httpUpdate.update(myLocalConn, fwImageURL);
        
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
          logSerialPrintln("[UPDATE] ✗ Aggiornamento FALLITO");
          Ntcurr.setText("U_F");
          delay(50);

          break;
        case HTTP_UPDATE_NO_UPDATES:
          logSerialPrintln("[UPDATE] Nessun aggiornamento disponibile");
          Ntcurr.setText("N_U");
          delay(50);

          break;
        case HTTP_UPDATE_OK:
          logSerialPrintln("[UPDATE] ✓ Aggiornamento completato - Riavvio...");
          // Riavvio automatico
          break;
        }
      }
      else
      {
        logSerialPrintln("[UPDATE] Firmware già aggiornato");
        Ntcurr.setText("S_V");
        delay(50);

      }
    }
    else
    {
      logSerialPrintf("[UPDATE] ✗ Errore HTTP: %d\n", httpCode);
      Ntcurr.setText("A_E");
      delay(50);

    }
    
    httpClient.end();
    myLocalConn.stop();
  }

  // ========== CALLBACK MQTT ==========
  void callback(char *topic, byte *payload, unsigned int length)
  {
#ifdef DEBUGMIO
    logSerialPrint("[CALLBACK] Message arrived [");
    logSerialPrint(topic);
    logSerialPrint("] ");
    for (int i = 0; i < length; i++)
    {
      logSerialPrint((char)payload[i]);
    }
    logSerialPrintln();
#else
    logSerialPrintf("[CALLBACK] Ricevuto da topic: %s\n", topic);
    
    delay(20);
    bool handled = false;

    // ========== GESTIONE COMANDI DI SISTEMA ==========
    if (strcmp(topic, systemTopic) == 0)
    {
      if (char(payload[0]) == '0')
      {
        handled = true;
        logSerialPrintln("[CALLBACK] Comando SLEEP ricevuto");
        adessoDormo(8, COMANDO_SYSTEM_TOPIC);
        return; // Esci immediatamente dopo dormire
      }
    }
    else if (strcmp(topic, updateTopic) == 0)
    {
      if (char(payload[0]) == '0')
      {
        handled = true;
        logSerialPrintln("[CALLBACK] Comando UPDATE ricevuto");
        Ntcurr.setText("UPMQ");
        delay(50);

        checkForUpdates();
        return;
      }
    }
    else if (strcmp(topic, acquaTopic) == 0)
    {
      handled = true;
      db_array_value[2] = (char(payload[0]) == '0') ? 0 : 1;
      Nwater_on.setPic(db_array_value[2]);
      logSerialPrintf("[CALLBACK] Stato acqua aggiornato: %d\n", db_array_value[2]);
    }
    else if (strcmp(topic, riscaldaTopic) == 0)
    {
      handled = true;
      db_array_value[1] = (char(payload[0]) == '0') ? 0 : 1;
      Nrisc_on.setPic(db_array_value[1]);
      logSerialPrintf("[CALLBACK] Stato riscaldamento aggiornato: %d\n", db_array_value[1]);
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
      logSerialPrintln("[CALLBACK] JSON non valido, ignoro messaggio");
      return;
    }

    String msg_Topic = root["topic"];
    logSerialPrintf("[CALLBACK] Topic JSON interno: %s\n", msg_Topic.c_str());

    // ========== GESTIONE MESSAGGI PER TOPIC ==========
    if (strcmp(topic, systemTopic) == 0)
    {
      if (msg_Topic == "UpTime")
      {
        const char *Nex_Time = root["hours"];
        
        Ncurr_hour.setText(Nex_Time);
        delay(50);
        const char *Nex_Day = root["Day"];
        Nday.setText(Nex_Day);
        delay(50);

        logSerialPrintf("[CALLBACK] Aggiornato orario: %s, giorno: %s\n", Nex_Time, Nex_Day);
      }
    }
    else if (strcmp(topic, casaSensTopic) == 0)
    {
      if (msg_Topic == "DHTCamera")
      {
        const char *Nex_inHm = root["Hum"];
        const char *Nex_inTemp = root["Temp"];
        Ntcurr.setText(Nex_inTemp);
        delay(50);

        Nin_hum.setText(Nex_inHm);
        delay(50);

        logSerialPrintf("[CALLBACK] DHT Camera - Temp: %s°C, Hum: %s%%\n", Nex_inTemp, Nex_inHm);
      }
    }
    else if (strcmp(topic, extSensTopic) == 0)
    {
      if (msg_Topic == "Caldaia")
      {
        float acquaTemp = root["acqua"];
        char tempStr[10];
        dtostrf(acquaTemp, 4, 1, tempStr);
        Ncurr_water_temp.setText(tempStr);
        delay(50);

        logSerialPrintf("[CALLBACK] Temp acqua caldaia: %s°C\n", tempStr);
    }
      else if (msg_Topic == "Terrazza")
      {
        float outHum = root["Hum"];
        float outTemp = root["Temp"];
        
        char humStr[10], tempStr[10];
        dtostrf(outHum, 4, 1, humStr);
        dtostrf(outTemp, 4, 2, tempStr);
        
        Nout_temp.setText(tempStr);
        delay(50);

        Nout_hum.setText(humStr);
        delay(50);

        logSerialPrintf("[CALLBACK] Terrazza - Temp: %s°C, Hum: %s%%\n", tempStr, humStr);
    }
    }
    else if (strcmp(topic, eneValTopic) == 0)
    {
      if (msg_Topic == "EneMain")
    {
        int eneVal = root["e"];  // se è un intero
        char eneStr[10];
        itoa(eneVal, eneStr, 10);
        Nset_temp.setText(eneStr);
        delay(50);

        logSerialPrintf("[CALLBACK] Valore energia: %s\n", eneStr);
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