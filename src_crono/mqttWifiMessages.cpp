
#include "mqttWifiMessages.h"
#include "ArduinoJson.h"
#include "NexManager.h"
#include "impostazioni.h"
#include "mqttWifi.h" // Include il file con connessioni
namespace mqttWifi {

// ========== INVIO DATI SENSORI ==========
void setCallback() { client.setCallback(callback); }
void sendData() {
  StaticJsonDocument<256> doc;

  doc["topic"] = "SalChr";
  doc["hum"] = myTemp.h / 4;  // raw value è x4
  doc["temp"] = myTemp.t / 4; // raw value è x4
  doc["cf"] = myTemp.confort;

  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer)); // ✅ safe con size limit

  LOG_VERBOSE("[SENDDATA] Invio dati temperatura %s\n", buffer);

  if (publish(casaSensTopic, buffer, true))
    LOG_INFO("[SENDDATA] ✓ Dati inviati con successo");
  else
    LOG_ERROR("[SENDDATA] ✗ Invio fallito");
}

// ========== AGGIORNAMENTO FIRMWARE ==========
void checkForUpdates() {
  LOG_VERBOSE("[UPDATE] Controllo aggiornamenti firmware...");

  String fwURL = String(fwUrlBase);
  fwURL.concat(chronoId);
  String fwVersionURL = fwURL + "/version.php";
  String fwImageURL = fwURL + "/firmware.bin";

  WiFiClient myLocalConn;
  HTTPClient httpClient;
  httpClient.begin(myLocalConn, fwVersionURL);

  int httpCode = httpClient.GET();
  if (httpCode == 200) {
    String newFWVersion = httpClient.getString();
    int newVersion = newFWVersion.toInt();

    LOG_VERBOSE("[UPDATE] Versione corrente: %d, Versione disponibile: %d\n",
                versione, newVersion);
    NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", newFWVersion.c_str());
    // Ntcurr.setText(newFWVersion.c_str());
    delay(1000);

    if (newVersion > versione) {
      LOG_VERBOSE("[UPDATE] Nuova versione disponibile! Avvio update...");

      client.disconnect();
      delay(1000);

      t_httpUpdate_return ret = httpUpdate.update(myLocalConn, fwImageURL);

      switch (ret) {
      case HTTP_UPDATE_FAILED:
        LOG_VERBOSE("[UPDATE] ✗ Aggiornamento FALLITO");
        NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "U_F");
        // Ntcurr.setText("U_F");
        delay(50);

        break;
      case HTTP_UPDATE_NO_UPDATES:
        LOG_VERBOSE("[UPDATE] Nessun aggiornamento disponibile");
        NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "N_U");
        // Ntcurr.setText("N_U");
        delay(50);

        break;
      case HTTP_UPDATE_OK:
        LOG_VERBOSE("[UPDATE] ✓ Aggiornamento completato - Riavvio...");
        // Riavvio automatico
        break;
      }
    } else {
      LOG_VERBOSE("[UPDATE] Firmware già aggiornato");
      NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "S_V");
      // Ntcurr.setText("S_V");
      delay(50);
    }
  } else {
    LOG_VERBOSE("[UPDATE] ✗ Errore HTTP: %d\n", httpCode);
    NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "A_E");
    // Ntcurr.setText("A_E");
    delay(50);
  }

  httpClient.end();
  myLocalConn.stop();
}

// ========== CALLBACK MQTT ==========
void callback(char *topic, byte *payload, unsigned int length) {
#ifdef DEBUGMIO
  logSerialPrint("[CALLBACK] Message arrived [");
  logSerialPrint(topic);
  logSerialPrint("] ");
  for (int i = 0; i < length; i++) {
    logSerialPrint((char)payload[i]);
  }
  LOG_VERBOSE();
#else
  LOG_VERBOSE("[CALLBACK] Ricevuto da topic: %s\n", topic);

  /* -------------------------------------------------------------
         1️⃣  COMANDI DI SISTEMA
      ------------------------------------------------------------- */
  if (strcmp(topic, systemTopic) == 0 && char(payload[0]) == '0') {
    LOG_VERBOSE("[CALLBACK] Comando SLEEP ricevuto");
    adessoDormo(8, COMANDO_SYSTEM_TOPIC);
    return;
  }
  if (strcmp(topic, updateTopic) == 0 && char(payload[0]) == '0') {
    LOG_VERBOSE("[CALLBACK] Comando UPDATE ricevuto");
    NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", "UPMQ");
    delay(50);
    checkForUpdates();
    return;
  }

  /* -------------------------------------------------------------
     2️⃣  COMANDI RELAY — fix: indici corretti
  ------------------------------------------------------------- */
  if (strcmp(topic, acquaTopic) == 0) {
    bool on = (char(payload[0]) != '0');
    stato.relays[ACQUA] = on; // stato aggiornato
    NexManager::sendFormatted("%s.picc=%d", "Nwater_on",
                              on); // display aggiornato
    return;
  }
  if (strcmp(topic, riscaldaTopic) == 0) {
    bool on = (char(payload[0]) != '0');
    stato.relays[RISCALDAMENTO] = on;
    NexManager::sendFormatted("%s.picc=%d", "Nrisc_on", on);
    return;
  }

  /* -------------------------------------------------------------
     3️⃣  FEEDBACK TENDE (BINARIO) — Nuovo protocollo Node-RED
  ------------------------------------------------------------- */
  if (strcmp(topic, tendeTuyaState) == 0) {
    if (length < 4 || payload[0] != 0xAA) {
      LOG_WARN(
          "[CALLBACK] Tende BIN: Header errato o length insufficiente (%d)\n",
          length);
      return;
    }

    uint8_t numTende = payload[2];
    uint8_t expectedChecksumIdx = 3 + (numTende * 2);

    // Verifica che il buffer contenga almeno i dati dichiarati + checksum
    if (length <= expectedChecksumIdx) {
      LOG_VERBOSE("[CALLBACK] Tende BIN: Payload corto per %d tende\n",
                  numTende);
      return;
    }

    // Verifica Checksum
    uint8_t calcChecksum = 0;
    for (unsigned int i = 0; i < expectedChecksumIdx; i++) {
      calcChecksum ^= payload[i];
    }

    if (calcChecksum != payload[expectedChecksumIdx]) {
      LOG_VERBOSE("[CALLBACK] Tende BIN: Checksum KO (calc:%02X, recv:%02X)\n",
                  calcChecksum, payload[expectedChecksumIdx]);
      return;
    }

    // Processamento dati
    uint8_t movedMask = 0;
    for (int i = 0; i < numTende; i++) {
      int baseIdx = 3 + (i * 2);
      uint8_t id = payload[baseIdx];
      uint8_t pos = payload[baseIdx + 1];

      if (id < 5) {
        // Confronto con la memoria: se la posizione è cambiata davvero
        if (stato.pos[id] != pos) {
          movedMask |= (1 << id);
          stato.pos[id] = (uint8_t)pos;

          // Se siamo già in pagina 1, aggiorniamo solo il valore dello slider
          if (stato.currPage == 1) {
            const char *barNames[] = {"ts_bar", "tl_bar", "ps_bar", "pl_bar",
                                      "pc_bar"};
            NexManager::sendFormatted("%s.val=%d", barNames[id], pos);
          }
          LOG_VERBOSE("[CALLBACK] Tenda %d feedback BIN: %d%%\n", id, pos);
        }
      }
    }

    // Cambiamo pagina solo se c'è stato movimento e non siamo già lì
    if (movedMask > 0 && stato.currPage != 1) {
      stato.selectionMask =
          movedMask;            // Prepariamo la maschera per la visualizzazione
      NexManager::setPage("1"); // Cambio pagina e refresh automatico
    }
    return;
  }

  /* -------------------------------------------------------------
     4️⃣  PARSING JSON — ArduinoJson v6, con length
  ------------------------------------------------------------- */
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    LOG_VERBOSE("[CALLBACK] JSON non valido: %s\n", err.c_str());
    return;
  }

  const char *innerTopic = doc["topic"];
  if (!innerTopic) {
    LOG_VERBOSE("[CALLBACK] JSON senza campo \"topic\" → ignorato");
    return;
  }

  LOG_VERBOSE("[CALLBACK] Topic JSON interno: %s\n", innerTopic);

  /* -------------------------------------------------------------
     4️⃣  DISPATCH PER TOPIC
  ------------------------------------------------------------- */
  if (strcmp(topic, systemTopic) == 0) {
    if (strcmp(innerTopic, "UpTime") == 0) {
      const char *Nex_Time = doc["hours"] | "N/A";
      const char *Nex_Day = doc["Day"] | "N/A";
      // ── aggiorna sempre lo stato ──────────────────────────────
      strlcpy(stato.timeStr, Nex_Time, sizeof(stato.timeStr));
      strlcpy(stato.dayStr, Nex_Day, sizeof(stato.dayStr));

      // ── aggiorna display solo se page 0 è visibile ────────────
      if (stato.currPage == 0) {
        NexManager::sendFormatted("%s.txt=\"%s\"", "Ncurr_hour", stato.timeStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nday", stato.dayStr);
      }
      LOG_VERBOSE("[CALLBACK] Orario: %s, Giorno: %s\n", stato.timeStr,
                  stato.dayStr);
    }
  } else if (strcmp(topic, casaSensTopic) == 0) {
    if (strcmp(innerTopic, "DHTCamera") == 0) {
      // ── aggiorna sempre lo stato ──────────────────────────────
      stato.temps[INT] = doc["Temp"] | 0.0f;
      stato.hums[INT] = doc["Hum"] | 0.0f;

      // ── aggiorna display solo se page 0 è visibile ────────────
      if (stato.currPage == 0) {
        char tempStr[10], humStr[10];
        dtostrf(stato.temps[INT], 4, 1, tempStr);
        dtostrf(stato.hums[INT], 4, 1, humStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Ntcurr", tempStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nin_hum", humStr);
        LOG_VERBOSE("[CALLBACK] DHT Camera - Temp: %s°C, Hum: %s%%\n", tempStr,
                    humStr);
      }
    }
  } else if (strcmp(topic, extSensTopic) == 0) {
    if (strcmp(innerTopic, "Caldaia") == 0) {
      // ── aggiorna sempre lo stato ──────────────────────────────
      stato.waterTemp = doc["acqua"] | 0.0f;

      // ── aggiorna display solo se page 0 è visibile ────────────
      if (stato.currPage == 0) {
        char tempStr[10];
        dtostrf(stato.waterTemp, 4, 1, tempStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nwater_temp", tempStr);
        LOG_VERBOSE("[CALLBACK] Temp acqua: %s°C\n", tempStr);
      }
    } else if (strcmp(innerTopic, "Terrazza") == 0) {
      // ── aggiorna sempre lo stato ──────────────────────────────
      stato.temps[EXT] = doc["Temp"] | 0.0f;
      stato.hums[EXT] = doc["Hum"] | 0.0f;

      // ── aggiorna display solo se page 0 è visibile ────────────
      if (stato.currPage == 0) {
        char tempStr[10], humStr[10];
        dtostrf(stato.temps[EXT], 4, 1, tempStr);
        dtostrf(stato.hums[EXT], 4, 1, humStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nout_temp", tempStr);
        NexManager::sendFormatted("%s.txt=\"%s\"", "Nout_hum", humStr);
        LOG_VERBOSE("[CALLBACK] Terrazza - Temp: %s°C, Hum: %s%%\n", tempStr,
                    humStr);
      }
    }
  } else if (strcmp(topic, eneValTopic) == 0) {
    if (strcmp(innerTopic, "EneMain") == 0) {
      // ── aggiorna sempre lo stato ──────────────────────────────
      stato.powerW = (uint16_t)(doc["e"] | 0);

      // ── aggiorna display solo se page 0 è visibile ────────────
      if (stato.currPage == 0) {
        NexManager::sendFormatted("%s.txt=\"%d\"", "Nset_temp", stato.powerW);
        LOG_VERBOSE("[CALLBACK] Valore energia: %d W\n", stato.powerW);
      }
    }
  }
#endif
}

} // namespace mqttWifi