#include "PageHandlers.h"
// #include <EspNowManager.h>
#include "mqttWifi.h"
#include <impostazioni.h>
// #include "mqttWiFiMessages.h"
#include "topic.h"

// Forward declaration di funzioni esterne se necessarie
namespace mqttWifi {
// ========== INVIO COMANDI TENDE ==========
void pubTende(Tende tendeTargets[], size_t numTende, ComandoTende comando,
              int percentuale) {
  for (size_t i = 0; i < numTende; i++) {
    StaticJsonDocument<128> doc;

    doc["t"] = (int)tendeTargets[i];
    doc["c"] = (int)comando;

    // Nota: PARZIALE rimosso da enum, gestito solo via feedback o se
    // reintrodotto if (comando == PARZIALE) { doc["p"] = percentuale; }

    char buffer[128];
    serializeJson(doc, buffer, sizeof(buffer)); // ✅ safe con size limit

    LOG_VERBOSE("[pubTende] Invio comando tenda %d...", (int)tendeTargets[i]);

    // Comandi tende: retained=false (sono comandi temporanei)
    if (publish(tendeTuyaCmd, buffer, false)) {
      LOG_VERBOSE("[pubTende] ✓ Comando inviato");
    } else {
      LOG_VERBOSE("[pubTende] ✗ Invio fallito");
    }

    if (numTende > 1) {
      delay(50);
    }
  }
}
void pubCommand(const char *f_topic, const char *f_value) {

  // Comandi rele': retained=false (sono comandi temporanei)
  if (publish(f_topic, f_value, false)) {
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
    if (evt.event == 1) {
      stato.currPage = 1;
      stato.selectionMask = 0x1F;
      NexManager::setPage("1");
      LOG_VERBOSE("[Page] Tende button - ALL OPEN\n");
    }
    break;
  case 8: // Tende Button ,ALL CLOSE
    if (evt.event == 1) {
      stato.currPage = 1;
      stato.selectionMask = 0x1F;
      NexManager::setPage("1");
      LOG_VERBOSE("[Page] Tende Button, ALL CLOSE\n");
    }
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
    stato.currPage = 1;
    stato.selectionMask = 0x1F;
    NexManager::setPage("1");
    LOG_VERBOSE("[Page0] Switching to Tende page\n");
    break;

  default:
    break;
  }
}

// Handler Pagina Tende (ID 1)
void handleTendePage(const NexManager::TouchEvent &evt) {
  // static uint32_t lastInteractionTime = 0;
  // const uint32_t TIMEOUT_PAGE = 15000;

  // if (lastInteractionTime == 0)
  //   lastInteractionTime = millis();

  // if (evt.isValid) {
  //   //lastInteractionTime = millis();
  //   LOG_VERBOSE("[Tende] Evento valido: %d. Timer resettato.\n", evt.event);
  // }else {
  //   LOG_VERBOSE("[Tende] Evento non valido: %d. Timer non resettato.\n", evt.event);
  // }

  // Verifica timeout per ritorno a casa
  // if (millis() - lastInteractionTime > TIMEOUT_PAGE) {
  //   LOG_VERBOSE("[Tende] Timeout! Ritorno alla Home\n");
  //   stato.currPage = 0;
  //   lastInteractionTime = 0;
  //   NexManager::setPage("0");
  //   return;
  // }

  if (evt.event == 1 && ) {
    bool eventConsumed = false;
    LOG_VERBOSE("[Tende] Evento valido: %d\n", evt.event);
    const char *barNames[] = {"pl_bar", "tl_bar", "ps_bar", "ts_bar", "pc_bar"};

    // 1. Selezione tende (ID 10-14, i crop)
    if (evt.component >= 10 && evt.component <= 14) {
      int id = evt.component - 10;
      
      // Se erano tutte selezionate (stato iniziale), seleziona SOLO quella premuta.
      // Altrimenti aggiungi alla selezione attuale.
      if (stato.selectionMask == 0x1F) {
        stato.selectionMask = (1 << id);
      } else {
        stato.selectionMask |= (1 << id);
      }

      LOG_VERBOSE("[Tende] Selezione aggiornata: %d (Mask: %02X)\n", id,
                  stato.selectionMask);
      eventConsumed = true;

      for (int i = 0; i < 5; i++) {
        bool isSelected = (stato.selectionMask & (1 << i));
        NexManager::sendFormatted("vis %s,%d", barNames[i], isSelected ? 1 : 0);
      }
    }

    // 2. Comandi movimento (Pulsanti 15, 16, 17)
    else if (evt.component >= 15 && evt.component <= 17) {
      if (stato.selectionMask == 0) {
        LOG_VERBOSE("[Tende] Nessuna tenda selezionata!\n");
        return;
      }
      eventConsumed = true;
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

      Tende targets[5];
      size_t count = 0;
      for (int i = 0; i < 5; i++) {
        if (stato.selectionMask & (1 << i)) {
          targets[count++] = (Tende)i;
        }
      }
      mqttWifi::pubTende(targets, count, cmd, 0);
    }
    if (!eventConsumed) {
      LOG_VERBOSE("[Tende] Evento non consumato: %d\n", evt.event);
    }
  }
}

void handleSettingsPage(const NexManager::TouchEvent &evt) {}