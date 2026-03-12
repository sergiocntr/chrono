#include "PageHandlers.h"
// #include <EspNowManager.h>
#include <impostazioni.h>
#include "mqttWifi.h"
// #include "mqttWiFiMessages.h"
#include "topic.h"

// Forward declaration di funzioni esterne se necessarie
namespace mqttWifi
{
    // ========== INVIO COMANDI TENDE ==========
    void pubTende(Tende tendeTargets[], size_t numTende, ComandoTende comando,
                  int percentuale)
    {
        StaticJsonDocument<256> doc;

        doc["comando"] = (int)comando;

        if (comando == 2) // APRI PARZIALE
        {
            doc["percentuale"] = percentuale;
        }

        JsonArray tendeArray = doc.createNestedArray("tende");
        for (size_t i = 0; i < numTende; i++)
        {
            tendeArray.add((int)tendeTargets[i]);
        }

        char buffer[256];
        serializeJson(doc, buffer, sizeof(buffer)); // ✅ safe con size limit

        LOG_VERBOSE("[pubTende] Invio comando tende...");

        // Comandi tende: retained=false (sono comandi temporanei)
        if (publish(tendeTuya, buffer, false))
        {
            LOG_VERBOSE("[pubTende] ✓ Comando inviato");
        }
        else
        {
            LOG_VERBOSE("[pubTende] ✗ Invio fallito");
        }
    }
    void pubCommand(const char *f_topic, const char *f_value)
    {

        // Comandi rele': retained=false (sono comandi temporanei)
        if (publish(f_topic, f_value, false))
        {
            LOG_VERBOSE("[pubCommand] ✓ Comando inviato");
        }
        else
        {
            LOG_VERBOSE("[pubCommand] ✗ Invio fallito");
        }
    }
}

// Handler Pagina Home (ID 0)
void handleHomePage(const NexManager::TouchEvent &evt)
{
    switch (evt.component)
    {
    case 5: // Hot Water
        // Non invertiamo stato.relays qui, non aggiorniamo il display
        // Mandiamo solo il comando MQTT con lo stato invertito
        mqttWifi::pubCommand(acquaTopic, stato.relays[ACQUA] ? "0" : "1");
        LOG_VERBOSE("[Page0] Hot Water\n");
        break;

    case 7: // Tende button - ALL OPEN
        if (evt.event == 1)
        {
            stato.currPage = 1;
            Tende allTende[] = {TENDA_SALOTTO, TENDA_LEO,
                                TAPPA_SALOTTO, TAPPA_LEO, TAPPA_CAMERA};
            mqttWifi::pubTende(allTende, 5, TENDE_STATUS, 0);
            LOG_VERBOSE("[Page] Tende button - ALL OPEN\n");
        }
        break;
    case 8: // Tende Button ,ALL CLOSE
        LOG_VERBOSE("[Page0] Tende Button ,ALL CLOSE\n");
        break;

    case 10: // Riscaldamento
             // Non invertiamo stato.relays qui, non aggiorniamo il display
             // Mandiamo solo il comando MQTT con lo stato invertito
        mqttWifi::pubCommand(riscaldaTopic, stato.relays[RISCALDAMENTO] ? "0" : "1");
        LOG_VERBOSE("[Page0] Riscaldamento\n");
        break;

    case 11: // shut down
        stato.relays[ALLARME] = !stato.relays[ALLARME];

        // Aggiorna il display
        NexManager::sendFormatted("%s.picc=%d", "Nalarm", stato.relays[ALLARME]);

        // Pubblica il nuovo stato via MQTT
        if (!stato.relays[ALLARME])
            mqttWifi::pubCommand(teleTopic, "spegni");
        LOG_VERBOSE("[Page0] ShutDown\n");
        break;

    case 12:                                            // home page
        LOG_VERBOSE("[Page0] already home page\n"); // nothing
        break;

    case 13:                                                  // curtain page
        LOG_VERBOSE("[Page0] Switching to Tende page\n"); // store value in stato
        // switch to page 1
        break;

    default:
        // Altri componenti della Home page
        break;
    }
}

// Handler Pagina Tende (ID 1)
void handleTendePage(const NexManager::TouchEvent &evt)
{
    static uint32_t lastCommandTime = 0;

    // Anti-spam: almeno 200ms tra comandi
    if (millis() - lastCommandTime < 200 && evt.event == 1)
        return;

    if (evt.event == 0)
    { // Solo press events
        // Selezione tenda (componenti 13-17)
        if (evt.component >= 13 && evt.component <= 17)
        {
            int indexMap[] = {3, 1, 2, 0, 4};
            stato.activeTenda = indexMap[evt.component - 13];
            lastCommandTime = millis();
            LOG_VERBOSE("[Tende] Selected: %d\n", stato.activeTenda);

            // Feedback visivo selezione
            NexManager::sendFormatted("vis q%d,1", stato.activeTenda);
        }

        // Comandi movimento
        if (stato.activeTenda != -1)
        {
            ComandoTende cmd = FERMA;
            const char *cmdName = "STOP";

            if (evt.component == 11)
            { // UP
                cmd = APRI;
                cmdName = "UP";
            }
            else if (evt.component == 12)
            { // DOWN
                cmd = CHIUDI;
                cmdName = "DOWN";
            }

            if (evt.component == 11 || evt.component == 12 || evt.component == 8)
            {
                lastCommandTime = millis();
                LOG_VERBOSE("[Tende] Command %s on %d\n",
                                cmdName, stato.activeTenda);

                // Feedback visivo (flash)
                NexManager::sendFormatted("click q%d,1", stato.activeTenda);
                delay(80);
                NexManager::sendFormatted("click q%d,0", stato.activeTenda);

                // Invia comando MQTT
                Tende target[] = {(Tende)stato.activeTenda};
                mqttWifi::pubTende(target, 1, cmd, 0);
            }
        }
    }
}

// Handler Pagina Impostazioni (ID 2 - esempio futuro)
void handleSettingsPage(const NexManager::TouchEvent &evt)
{
    // Implementazione per pagina impostazioni
    // ...
}