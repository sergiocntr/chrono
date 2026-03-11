#include "mqttWifi.h"


WiFiClient mywifi;
WiFiClient c;

namespace mqttWifi
{
  PubSubClient client(c);

  // ========== CONFIGURAZIONE ==========
  const uint8_t MAX_TENTATIVI = 3;
  const unsigned long TIMEOUT_WIFI = 12000;
  const unsigned long TIMEOUT_MQTT = 12000;
  const unsigned long SLEEP_TIME_US = 5ULL * 60ULL * 1000000ULL;

  uint8_t tentativiWifi = 0;
  uint8_t tentativiMqtt = 0;

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
        logSerialPrintf("[PUBLISH] OK su tentativo %d %s\n", tentativo + 1,
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

  // ========== LOG MOTIVO SPEGNIMENTO ==========
  void logMotivoSpegnimento(MotivoSpegnimento motivo)
  {
    logSerialPrint("[SLEEP] Motivo: ");
    switch (motivo)
    {
    case PUBLISH_FALLITO:
      logSerialPrintln("PUBLISH FALLITO dopo 3 tentativi");
      break;
    case COMANDO_SYSTEM_TOPIC:
      logSerialPrintln("COMANDO via systemTopic (payload '0')");
      break;
    case WIFI_TIMEOUT_CONNESSIONE:
      logSerialPrintln("WiFi TIMEOUT dopo 3 tentativi");
      break;
    case MQTT_TIMEOUT_CONNESSIONE:
      logSerialPrintln("MQTT TIMEOUT dopo 3 tentativi");
      break;
    case WIFI_FALLITO_SETUP:
      logSerialPrintln("WiFi FALLITO durante setup");
      break;
    case NEXTION_SETUP_FAILED:
      logSerialPrintln("NEXTION INIT FAILLITO durante setup");
      break;
    case DHT_SETUP_FAILED:
      logSerialPrintln("DHT INIT FAILLITO durante setup");
      break;
    case SHUTDOWN_FROM_MQTT:
      logSerialPrintln("SHUTDOWN FROM MQTT");
      break;
    default:
      logSerialPrintln("SCONOSCIUTO");
      break;
    }
    Serial.flush();
  }

  // ========== DEEP SLEEP ==========
  void adessoDormo(uint8_t mode, MotivoSpegnimento motivo)
  {
    logMotivoSpegnimento(motivo);

    // Spegnimento Nextion
    if (mode > 0)
    {
      logSerialPrintln("[SLEEP] Spegnimento Nextion");
      //sendCommand("thup=1");
      //sendCommand("sleep=1");
      delay(200);
    }

    // Chiusura MQTT
    if (client.connected())
    {
      logSerialPrintln("[SLEEP] Disconnessione MQTT");
      client.disconnect();
      delay(100);
    }

    // Spegnimento WiFi
    logSerialPrintln("[SLEEP] Spegnimento WiFi");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(200);

    // Deep sleep
    logSerialPrintln("[SLEEP] Deep sleep per 5 minuti");
    Serial.flush();
    delay(100);
#ifdef ESP8266_BUILD
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    WiFi.forceSleepBegin();
    delay(300000);
    ESP.reset();

#elif ESP32_BUILD
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
    esp_deep_sleep_start(); // NON ritorna mai
#endif
  }

  // ========== SETUP WiFi ==========
  void setupWifi()
  {
    WiFi.persistent(false);
    WiFi.disconnect(true);

    WiFi.mode(WIFI_OFF);
    delay(200);

    WiFi.mode(WIFI_STA);
    delay(200);

    WiFi.setAutoReconnect(false);
    WiFi.setSleep(false);
#ifdef ESP8266_BUILD
    WiFi.setOutputPower(17);
    WiFi.forceSleepWake();
    logSerialPrintln("[WiFi] Setup ESP8266");

#elif ESP32_BUILD
    logSerialPrintln("[WiFi] Setup ESP32 C3");
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

#endif

    WiFi.config(ipChrono, gateway, subnet, dns1);
  }

  void randomDelayAtBoot()
  {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    unsigned long delayMs = ((mac[4] + mac[5]) % 100) * 10;
    logSerialPrintf("[SETUP] Delay casuale: %lu ms\n", delayMs);
    delay(delayMs);
  }

  // ========== CONNESSIONE WiFi CON RETRY ==========
  bool connectWifi()
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      tentativiWifi = 0; // Reset contatore se connesso
      return true;
    }

    logSerialPrintf("[WiFi] Tentativo %d/%d\n", tentativiWifi + 1, MAX_TENTATIVI);
    WiFi.begin(ssid, password);
    delay(200);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
      if (millis() - start > TIMEOUT_WIFI)
      {
        logSerialPrintln("[WiFi] TIMEOUT");
        tentativiWifi++;
        return false;
      }
      delay(250);
      logSerialPrint(".");
    }

    logSerialPrintln("\n[WiFi] ✓ Connesso");
    logSerialPrint("[WiFi] IP: ");
    logSerialPrintln(WiFi.localIP());

    tentativiWifi = 0; // Reset contatore
    delay(50);
    return true;
  }

  // ========== CONNESSIONE MQTT CON RETRY ==========
  bool connectMqtt()
  {
    if (client.connected())
    {
      tentativiMqtt = 0; // Reset contatore se connesso
      return true;
    }

    logSerialPrintf("[MQTT] Tentativo %d/%d\n", tentativiMqtt + 1, MAX_TENTATIVI);

    String clientId = String(mqttId) + String(random(0xffff), HEX);

    client.setServer(mqtt_server, mqtt_port);
    client.setBufferSize(512);

    uint32_t start = millis();
    while (!client.connected())
    {
      if (millis() - start > TIMEOUT_MQTT)
      {
        logSerialPrintln("[MQTT] TIMEOUT");
        tentativiMqtt++;
        return false;
      }

      if (client.connect(clientId.c_str(), mqttUser, mqttPass))
      {
        logSerialPrintln("[MQTT] ✓ Connesso");
        tentativiMqtt = 0; // Reset contatore
        return sottoscriviTopic();
      }

      delay(250);
      logSerialPrint(".");
    }

    return false;
  }

  // ========== SOTTOSCRIZIONE TOPIC ==========
  bool sottoscriviTopic()
  {
    logSerialPrintln("[MQTT] Sottoscrizione topic...");

    client.publish(logTopic, "Crono connesso");
    delay(10);

    bool success = true;
    success &= client.subscribe(systemTopic);
    success &= client.subscribe(casaSensTopic);
    success &= client.subscribe(extSensTopic);
    success &= client.subscribe(acquaTopic);
    success &= client.subscribe(riscaldaTopic);
    success &= client.subscribe(updateTopic);
    success &= client.subscribe(eneValTopic);

    client.loop();

    logSerialPrintln(success ? "[MQTT] ✓ Sottoscrizioni OK" : "[MQTT] ✗ Errore sottoscrizioni");
    return success;
  }

  // ========== GESTIONE CONNESSIONE (CHIAMARE NEL LOOP) ==========
  MotivoSpegnimento gestisciConnessione()
  {
    // Verifica WiFi
    if (WiFi.status() != WL_CONNECTED)
    {
      logSerialPrintln("[GESTIONE] WiFi disconnesso!");

      if (!connectWifi())
      {
        // Fallito questo tentativo
        if (tentativiWifi >= MAX_TENTATIVI)
        {
          logSerialPrintf("[GESTIONE] WiFi fallito dopo %d tentativi\n", MAX_TENTATIVI);
          //adessoDormo(8, WIFI_TIMEOUT_CONNESSIONE);
          return WIFI_TIMEOUT_CONNESSIONE; // Non necessario (deep sleep non ritorna), ma per chiarezza
        }
        tentativiWifi ++;
        return CONN_OK; // Riprova al prossimo ciclo
      }
    }

    // Verifica MQTT
    if (!client.connected())
    {
      logSerialPrintln("[GESTIONE] MQTT disconnesso!");

      if (!connectMqtt())
      {
        // Fallito questo tentativo
        if (tentativiMqtt >= MAX_TENTATIVI)
        {
          logSerialPrintf("[GESTIONE] MQTT fallito dopo %d tentativi\n", MAX_TENTATIVI);
          //adessoDormo(8, MQTT_TIMEOUT_CONNESSIONE);
          return MQTT_TIMEOUT_CONNESSIONE;
        }
        tentativiMqtt ++;
        return CONN_OK; // Riprova al prossimo ciclo
      }
    }

    // Tutto OK: mantieni la connessione
    client.loop();
    tentativiWifi = 0; tentativiMqtt = 0;
    return CONN_OK;
  }

  // ========== SETUP COMPLETO ==========
  MotivoSpegnimento setupCompleto()
  {
    logSerialPrintln("========================================");
    logSerialPrintln("[SETUP] Avvio mqttWifi");
    logSerialPrintln("========================================");

    setupWifi();
    randomDelayAtBoot();

    // Primo tentativo WiFi
    if (!connectWifi())
    {
      //adessoDormo(8, WIFI_FALLITO_SETUP);
      return WIFI_FALLITO_SETUP;
    }

    // Primo tentativo MQTT
    if (!connectMqtt())
    {
      //adessoDormo(8, MQTT_TIMEOUT_CONNESSIONE);
      return MQTT_TIMEOUT_CONNESSIONE;
    }

    logSerialPrintln("[SETUP] ✓ Setup completato con successo");
    return SETUP_OK;
  }

} // namespace mqttWifi