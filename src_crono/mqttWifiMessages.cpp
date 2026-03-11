
#include "mqttWifiMessages.h"
#include "NexManager.h"
#include "impostazioni.h"
#include "mqttWifi.h" // Include il file con connessioni
#include "ArduinoJson.h"
namespace mqttWifi
{

  // ========== INVIO DATI SENSORI ==========
  void setCallback()
  {
    client.setCallback(callback);
  }
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

      logSerialPrintf(
          "[UPDATE] Versione corrente: %d, Versione disponibile: %d\n", versione,
          newVersion);
      NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", newFWVersion.c_str());
      // Ntcurr.setText(newFWVersion.c_str());
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
          NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "U_F");
          // Ntcurr.setText("U_F");
          delay(50);

          break;
        case HTTP_UPDATE_NO_UPDATES:
          logSerialPrintln("[UPDATE] Nessun aggiornamento disponibile");
          NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "N_U");
          // Ntcurr.setText("N_U");
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
        NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "S_V");
        // Ntcurr.setText("S_V");
        delay(50);
      }
    }
    else
    {
      logSerialPrintf("[UPDATE] ✗ Errore HTTP: %d\n", httpCode);
      NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "A_E");
      // Ntcurr.setText("A_E");
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
    // bool handled = false;

    /* -------------------------------------------------------------
       1️⃣  GESTIONE COMANDI DI SISTEMA (sleep, update, …)
       ------------------------------------------------------------- */
    if (strcmp(topic, systemTopic) == 0 && char(payload[0]) == '0')
    {
      logSerialPrintln("[CALLBACK] Comando SLEEP ricevuto");
      adessoDormo(8, COMANDO_SYSTEM_TOPIC);
      return;
    }
    if (strcmp(topic, updateTopic) == 0 && char(payload[0]) == '0')
    {
      logSerialPrintln("[CALLBACK] Comando UPDATE ricevuto");
      NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "UPMQ");
      // Ntcurr.setText("UPMQ");
      delay(50);
      checkForUpdates();
      return;
    }

    /* -------------------------------------------------------------
       2️⃣  COMANDI RELAY (acqua / riscalda) → aggiornamento crop
       ------------------------------------------------------------- */
    if (strcmp(topic, acquaTopic) == 0)
    {
      bool on = (char(payload[0]) != '0');
      stato.relays[ACQUA] = on;
      NexManager::sendFormatted("%s.pic=%d", "Nwater_on", on);
      // setRelayCrop(acquaTopic, on); // gestisce Nwater_on
      return;
    }
    if (strcmp(topic, riscaldaTopic) == 0)
    {
      bool on = (char(payload[0]) != '0');
      stato.relays[ACQUA] = on;
      NexManager::sendFormatted("%s.pic=%d", "Nrisc_on", on);
      // setRelayCrop(riscaldaTopic, on); // gestisce Nrisc_on
      return;
    }

    /* -------------------------------------------------------------
       3️⃣  PARSING JSON GENERICO (sensori vari)
       ------------------------------------------------------------- */
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(payload);
    if (!root.success())
    {
      logSerialPrintln("[CALLBACK] JSON non valido, ignoro messaggio");
      return;
    }

    // La chiave obbligatoria "topic" indica il tipo di sensore
    const char *innerTopic = root["topic"];
    if (!innerTopic)
    {
      logSerialPrintln("[CALLBACK] JSON senza campo \"topic\" → ignorato");
      return;
    }

    // Aggiorna lo stato globale (temperatura, umidità, power, …)
    // updateSystemState(root);

    // Aggiornamento UI specifico (es. mostrare valori sul display)
    // handleGenericSensor(innerTopic, root);

    // (eventuale refresh della pagina corrente)
    // NexManager::refreshCurrentPage();

    /* -------------------------------------------------------------
       4️⃣  Mantieni la connessione MQTT viva
       ------------------------------------------------------------- */
    if (client.connected())
      client.loop();
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

        // Ncurr_hour.setText(Nex_Time);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Ncurr_hour", Nex_Time); // Testo

        // delay(50);
        const char *Nex_Day = root["Day"];
        // Nday.setText(Nex_Day);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nday", Nex_Day); // Testo
        // delay(50);

        logSerialPrintf("[CALLBACK] Aggiornato orario: %s, giorno: %s\n",
                        Nex_Time, Nex_Day);
      }
    }
    else if (strcmp(topic, casaSensTopic) == 0)
    {
      if (msg_Topic == "DHTCamera")
      {
        const char *Nex_inHm = root["Hum"];
        const char *Nex_inTemp = root["Temp"];
        // Ntcurr.setText(Nex_inTemp);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", Nex_inTemp); // Testo
        // delay(50);

        // Nin_hum.setText(Nex_inHm);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nin_hum", Nex_inHm); // Testo
        // delay(50);

        logSerialPrintf("[CALLBACK] DHT Camera - Temp: %s°C, Hum: %s%%\n",
                        Nex_inTemp, Nex_inHm);
      }
    }
    else if (strcmp(topic, extSensTopic) == 0)
    {
      if (msg_Topic == "Caldaia")
      {
        float acquaTemp = root["acqua"];
        char tempStr[10];
        dtostrf(acquaTemp, 4, 1, tempStr);
        // Nwater_temp.setText(tempStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nwater_temp", tempStr); // Testo

        // delay(50);

        logSerialPrintf("[CALLBACK] Temp acqua caldaia: %s°C\n", tempStr);
      }
      else if (msg_Topic == "Terrazza")
      {
        float outHum = root["Hum"];
        float outTemp = root["Temp"];

        char humStr[10], tempStr[10];
        dtostrf(outHum, 4, 1, humStr);
        dtostrf(outTemp, 4, 2, tempStr);

        // Nout_temp.setText(tempStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nout_temp", tempStr); // Testo
        // delay(50);

        // Nout_hum.setText(humStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nout_hum", humStr); // Testo
        // delay(50);

        logSerialPrintf("[CALLBACK] Terrazza - Temp: %s°C, Hum: %s%%\n", tempStr,
                        humStr);
      }
    }
    else if (strcmp(topic, eneValTopic) == 0)
    {
      if (msg_Topic == "EneMain")
      {
        int eneVal = root["e"]; // se è un intero
        char eneStr[10];
        itoa(eneVal, eneStr, 10);
        // Nset_temp.setText(eneStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nset_temp", eneStr); // Testo
        // delay(50);

        logSerialPrintf("[CALLBACK] Valore energia: %s\n", eneStr);
      }
    }

    // delay(50);
    if (client.connected())
    {
      client.loop();
    }
#endif
  }

} // namespace mqttWifi