#include "PageHandlers.h"
// #include <EspNowManager.h>
#include "mqttWifi.h"
#include <impostazioni.h>
// #include "mqttWiFiMessages.h"
#include "topic.h"

// Forward declaration di funzioni esterne se necessarie
namespace mqttWifi {
// ========== INVIO COMANDI TENDE ==========
void pubTende(ComandoTende comando) {
  if (stato.selectionMask == 0) {
    LOG_VERBOSE("[pubTende] Nessuna tenda selezionata!\n");
    return;
  }

  Tende targets[5];
  size_t count = 0;
  for (int i = 0; i < 5; i++) {
    if (stato.selectionMask & (1 << i)) {
      targets[count++] = (Tende)i;
    }
  }

  for (size_t i = 0; i < count; i++) {
    StaticJsonDocument<128> doc;
    doc["t"] = (int)targets[i];
    doc["c"] = (int)comando;

    char buffer[128];
    serializeJson(doc, buffer, sizeof(buffer));

    LOG_VERBOSE("[pubTende] Invio comando %d a tenda %d...", (int)comando,
                (int)targets[i]);

    if (publish(tendeTuyaCmd, buffer, false)) {
      LOG_VERBOSE(" ✓ Comando inviato");
    } else {
      LOG_VERBOSE(" ✗ Invio fallito");
    }

    if (count > 1) {
      delay(50);
    }
  }
}

void pubCommand(const char *f_topic, const char *f_value) {

  // Comandi rele': retained=false (sono comandi temporanei)
  if (mqttWifi::publish(f_topic, f_value, false)) {
    LOG_VERBOSE("[pubCommand] ✓ Comando inviato");
  } else {
    LOG_VERBOSE("[pubCommand] ✗ Invio fallito");
  }
}
} // namespace mqttWifi

// Handler Pagina Home (ID 0)
void handleHomePage(const NexManager::TouchEvent &evt) {
  switch (evt.component) {
  case 5: // Hot Water
    mqttWifi::pubCommand(acquaTopic, stato.relays[ACQUA] ? "0" : "1");
    LOG_VERBOSE("[Page0] Hot Water\n");
    break;

  case 7: // Tende button - ALL OPEN
    mqttWifi::pubTende(T_OPEN);

    LOG_VERBOSE("[Page] Tende button - ALL OPEN\n");

    break;
  case 8: // Tende Button ,ALL CLOSE

    mqttWifi::pubTende(T_CLOSE);

    LOG_VERBOSE("[Page] Tende Button, ALL CLOSE\n");

    break;

  case 10: // Riscaldamento
    mqttWifi::pubCommand(riscaldaTopic,
                         stato.relays[RISCALDAMENTO] ? "0" : "1");
    LOG_VERBOSE("[Page0] Riscaldamento\n");
    break;

  case 11: // shut down
    stato.relays[ALLARME] = !stato.relays[ALLARME];
    NexManager::sendFormatted("%s.picc=%d", "Nalarm", stato.relays[ALLARME]);
    if (!stato.relays[ALLARME])
      mqttWifi::pubCommand(teleTopic, "spegni");
    LOG_VERBOSE("[Page0] ShutDown\n");
    break;

  case 12: // home page
    LOG_VERBOSE("[Page0] already home page\n");
    break;

  case 13: // curtain page

    NexManager::setPage("1");
    LOG_VERBOSE("[Page0] Switching to Tende page\n");
    break;

  default:
    break;
  }
}

// --- Timer di inattività pagina Tende ---
// Variabile condivisa tra le due funzioni qui sotto.
static uint32_t s_tendeLastInteraction = 0;
static const uint32_t TENDE_TIMEOUT_MS = 15000;

// Resetta il timer: da chiamare all'ingresso in pagina 1 e su ogni touch.
void resetTendeTimer() { s_tendeLastInteraction = millis(); }

// Controlla se il timeout è scaduto: da chiamare ad ogni ciclo di smartDelay
// quando stato.currPage == 1. Torna in home autonomamente.
void checkTendeTimeout() {
  if (s_tendeLastInteraction == 0)
    return; // timer non ancora avviato

  if (millis() - s_tendeLastInteraction > TENDE_TIMEOUT_MS) {
    LOG_VERBOSE("[Tende] Timeout inattivita'! Ritorno alla Home\n");
    s_tendeLastInteraction = 0;
    NexManager::setPage("0");
  }
}

// Handler Pagina Tende (ID 1) — gestisce solo gli eventi touch.
// Il timeout è controllato da checkTendeTimeout() in smartDelay.
void handleTendePage(const NexManager::TouchEvent &evt) {
  // Qualsiasi touch sulla pagina 1 resetta il timer di inattivita'
  resetTendeTimer();

  const char *barNames[] = {"pl_bar", "tl_bar", "ps_bar", "ts_bar", "pc_bar"};

  // 1. Selezione tende (ID 10-14, i crop)
  if (evt.component >= 10 && evt.component <= 14) {
    int id = evt.component - 10;
    stato.selectionMask |= (1 << id);
    LOG_VERBOSE("[Tende] Selezione aggiunta: %d (Mask: %02X)\n", id,
                stato.selectionMask);

    for (int i = 0; i < 5; i++) {
      bool isSelected = (stato.selectionMask & (1 << i));
      NexManager::sendFormatted("vis %s,%d", barNames[i], isSelected ? 1 : 0);
    }
  }

  // 2. Comandi movimento (Pulsanti 15, 16, 17)
  if (evt.component >= 15 && evt.component <= 17) {
    if (stato.selectionMask == 0) {
      LOG_VERBOSE("[Tende] Nessuna tenda selezionata!\n");
      return;
    }

    ComandoTende cmd;
    const char *cmdName;
    switch (evt.component) {
    case 15:
      cmd = T_STOP;
      cmdName = "STOP";
      break;
    case 16:
      cmd = T_OPEN;
      cmdName = "OPEN";
      break;
    case 17:
      cmd = T_CLOSE;
      cmdName = "CLOSE";
      break;
    default:
      return;
    }

    LOG_VERBOSE("[Tende] Comando %s su maschera %02X\n", cmdName,
                stato.selectionMask);

    mqttWifi::pubTende(cmd);
  }
}
void handleSettingsPage(const NexManager::TouchEvent &evt) {}
