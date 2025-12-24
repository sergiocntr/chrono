#pragma once
#include "myIP.h"
#include "password.h"
#include "topic.h"
#include "mqttWifiMessages.h" 
WiFiClient mywifi;
WiFiClient c;

namespace mqttWifi
{
  PubSubClient client(c);

 // ========== VARIABILI DI STATO ==========
  bool mqttConnesso = false;
  bool inFaseRiposo = false;
  unsigned long timerSenzaConnessione = 0;
  const unsigned long INTERVALLO_RIPROVA = 300000; // 5 minuti
  const unsigned long TIMEOUT_WIFI = 7000;
  const unsigned long TIMEOUT_MQTT = 5000;

  // Forward declarations
  void adessoDormo(uint8_t mode, MotivoSpegnimento motivo);
  bool reconnectMqtt();

  // ========== FUNZIONE LOG MOTIVO ==========
  void logMotivoSpegnimento(MotivoSpegnimento motivo)
  {
    Serial.print("[SLEEP] Motivo: ");
    switch (motivo)
    {
    case PUBLISH_FALLITO:
      Serial.println("PUBLISH FALLITO dopo 3 tentativi");
      break;
    case COMANDO_SYSTEM_TOPIC:
      Serial.println("COMANDO via systemTopic (payload '0')");
      break;
    case WIFI_TIMEOUT_GESTIONE:
      Serial.println("WiFi TIMEOUT in gestisciConnessione()");
      break;
    case MQTT_TIMEOUT_GESTIONE:
      Serial.println("MQTT TIMEOUT in gestisciConnessione()");
      break;
    case WIFI_FALLITO_SETUP:
      Serial.println("WiFi FALLITO durante setupCompleto()");
      break;
    case MQTT_FALLITO_RISVEGLIO:
      Serial.println("MQTT FALLITO durante risveglio");
      break;
    case WIFI_FALLITO_RISVEGLIO:
      Serial.println("WiFi FALLITO durante risveglio");
      break;
    default:
      Serial.println("SCONOSCIUTO");
      break;
    }
    Serial.flush(); // Assicura che venga scritto prima dello spegnimento
  }

  // ========== GESTIONE MODALITÀ RIPOSO ==========
  void adessoDormo(uint8_t mode, MotivoSpegnimento motivo)
  {
    // LOG DEL MOTIVO
    logMotivoSpegnimento(motivo);

    // Spegnimento Nextion (se mode > 0)
    if (mode > 0)
    {
      Serial.println("[SLEEP] Spegnimento Nextion");
      sendCommand("thup=1");
      sendCommand("sleep=1");
    }

    // Disconnessione pulita
    if (client.connected())
    {
      Serial.println("[SLEEP] Disconnessione MQTT");
      client.disconnect();
    }
    
    Serial.println("[SLEEP] Spegnimento WiFi");
    WiFi.disconnect(true); // true = cancella credenziali salvate
    WiFi.mode(WIFI_OFF);

    // Imposta stato di riposo
    inFaseRiposo = true;
    mqttConnesso = false;
    timerSenzaConnessione = millis();
    
    Serial.printf("[SLEEP] Risveglio previsto tra %lu ms\n", INTERVALLO_RIPROVA);
    Serial.println("========================================");
  }

  // ========== SETUP INIZIALE ==========
  void setupWifi()
  {
    Serial.println("[SETUP] Configurazione WiFi...");
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
    Serial.printf("[SETUP] Delay casuale: %lu ms\n", delayMs);
    delay(delayMs);
  }

  // ========== CONNESSIONE WIFI ==========
  bool connectWifi()
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("[WiFi] Già connesso");
      return true;
    }

    Serial.println("[WiFi] Connessione in corso...");
    WiFi.begin(ssid, password);
    uint32_t start = millis();
    
    while (WiFi.status() != WL_CONNECTED)
    {
      if (millis() - start > TIMEOUT_WIFI)
      {
        Serial.printf("[WiFi] TIMEOUT dopo %lu ms\n", TIMEOUT_WIFI);
        return false;
      }
      delay(250);
      Serial.print(".");
    }

    Serial.println("\n[WiFi] Connesso!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  // ========== CONNESSIONE MQTT ==========
  bool connectMqtt()
  {
    if (client.connected())
    {
      Serial.println("[MQTT] Già connesso");
      return true;
    }

    String clientId = String(mqttId) + String(random(0xffff), HEX);
    Serial.printf("[MQTT] Connessione come: %s\n", clientId.c_str());
    
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback); // Callback definita in mqttWifiMessages.h
    client.setBufferSize(512); // Aumenta buffer se necessario

    uint32_t start = millis();
    while (!client.connected())
    {
      if (millis() - start > TIMEOUT_MQTT)
      {
        Serial.printf("[MQTT] TIMEOUT dopo %lu ms\n", TIMEOUT_MQTT);
        return false;
      }

      if (client.connect(clientId.c_str(), mqttUser, mqttPass))
      {
        Serial.println("[MQTT] Connesso!");
        return reconnectMqtt(); // Sottoscrivi ai topic
      }
      
      delay(250);
      Serial.print(".");
    }

    return false;
  }

  // ========== SOTTOSCRIZIONE TOPIC ==========
  bool reconnectMqtt()
  {
    if (!client.connected())
    {
      Serial.println("[MQTT] Client non connesso, impossibile sottoscrivere");
      return false;
    }

    Serial.println("[MQTT] Sottoscrizione topic...");
    
    // Pubblica messaggio di connessione
    client.publish(logTopic, "Crono connesso");
    delay(10);

    // Sottoscrivi a tutti i topic necessari
    bool success = true;
    
    success &= client.subscribe(systemTopic);
    Serial.printf("[MQTT] systemTopic: %s\n", success ? "OK" : "FAIL");
    delay(10);
    
    success &= client.subscribe(casaSensTopic);
    Serial.printf("[MQTT] casaSensTopic: %s\n", success ? "OK" : "FAIL");
    delay(10);
    
    success &= client.subscribe(extSensTopic);
    Serial.printf("[MQTT] extSensTopic: %s\n", success ? "OK" : "FAIL");
    delay(10);
    
    success &= client.subscribe(acquaTopic);
    Serial.printf("[MQTT] acquaTopic: %s\n", success ? "OK" : "FAIL");
    delay(10);
    
    success &= client.subscribe(riscaldaTopic);
    Serial.printf("[MQTT] riscaldaTopic: %s\n", success ? "OK" : "FAIL");
    delay(10);
    
    success &= client.subscribe(updateTopic);
    Serial.printf("[MQTT] updateTopic: %s\n", success ? "OK" : "FAIL");
    delay(10);
    
    success &= client.subscribe(eneValTopic);
    Serial.printf("[MQTT] eneValTopic: %s\n", success ? "OK" : "FAIL");
    
    client.loop(); // Completa handshake
    
    mqttConnesso = success;
    
    if (success)
    {
      Serial.println("[MQTT] ✓ Tutte le sottoscrizioni completate");
    }
    else
    {
      Serial.println("[MQTT] ✗ ERRORE in una o più sottoscrizioni");
    }
    
    return success;
  }

  // ========== RISVEGLIO DOPO RIPOSO ==========
  void risveglioE_Riconnetti()
  {
    Serial.println("[RISVEGLIO] Tentativo riconnessione...");
    WiFi.mode(WIFI_STA); // Riabilita WiFi
    
    if (connectWifi())
    {
      inFaseRiposo = false;
      
      // Riconnetti MQTT
      if (connectMqtt())
      {
        mqttConnesso = true;
        
        // Riaccendi Nextion
        Serial.println("[RISVEGLIO] Accensione Nextion");
        sendCommand("sleep=0");
        sendCommand("thup=0");
        Serial.println("[RISVEGLIO] ✓ Riconnessione completata!");
      }
      else
      {
        // MQTT fallito, torna a dormire
        Serial.println("[RISVEGLIO] ✗ MQTT fallito");
        adessoDormo(0, MQTT_FALLITO_RISVEGLIO); // 0 = non rispegnere Nextion
      }
    }
    else
    {
      // WiFi fallito, torna a dormire
      Serial.println("[RISVEGLIO] ✗ WiFi fallito");
      adessoDormo(0, WIFI_FALLITO_RISVEGLIO);
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
      Serial.println("[GESTIONE] WiFi disconnesso!");
      if (!connectWifi())
      {
        adessoDormo(8, WIFI_TIMEOUT_GESTIONE);
        return;
      }
    }

    // Verifica MQTT
    if (!client.connected())
    {
      Serial.println("[GESTIONE] MQTT disconnesso!");
      if (!connectMqtt())
      {
        adessoDormo(8, MQTT_TIMEOUT_GESTIONE);
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
    Serial.println("========================================");
    Serial.println("[SETUP] Avvio mqttWifi namespace");
    Serial.println("========================================");
    
    setupWifi();
    randomDelayAtBoot(); // Evita collisioni all'avvio
    
    if (connectWifi())
    {
      connectMqtt();
    }
    else
    {
      adessoDormo(8, WIFI_FALLITO_SETUP);
    }
  }

} // namespace mqttWifi