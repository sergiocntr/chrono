#include "PageHandlers.h"
// #include <EspNowManager.h>
#include <impostazioni.h>
#include "mqttWifi.h"
//#include "mqttWiFiMessages.h"
#include "topic.h"

// Forward declaration di funzioni esterne se necessarie
namespace mqttWifi
{
 // ========== INVIO COMANDI TENDE ==========
  void sendTende(Tende tendeTargets[], size_t numTende, ComandoTende comando,
                 int percentuale)
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
}

// Handler Pagina Home (ID 0)
void handleHomePage(const NexManager::TouchEvent &evt)
{
    switch (evt.component)
    {
    case 5: // Hot Water
    logSerialPrintf("[Page0] Hot Water\n");
        break;

    case 7: // Tende button - ALL OPEN
        if (evt.event == 1)
        {
            stato.currPage = 1;
            Tende allTende[] = {TENDA_SALOTTO, TENDA_LEO,
                                TAPPA_SALOTTO, TAPPA_LEO, TAPPA_CAMERA};
            mqttWifi::sendTende(allTende, 5, TENDE_STATUS, 0);
            logSerialPrintf("[Page] Tende button - ALL OPEN\n");
        }
        break;
    case 8: // Tende Button ,ALL CLOSE
        logSerialPrintf("[Page0] Tende Button ,ALL CLOSE\n");
        break;

    case 10: // Riscaldamento
        logSerialPrintf("[Page0] Riscaldamento\n");
        break;

    case 11: // shut down
        logSerialPrintf("[Page0] shut down\n");
        break;

    case 12:  // home page
        logSerialPrintf("[Page0] already home page\n"); // nothing
        break;

    case 13:                                                 // curtain page
        logSerialPrintf("[Page0] Switching to Tende page\n"); // store value in stato
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
            logSerialPrintf("[Tende] Selected: %d\n", stato.activeTenda);

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
                logSerialPrintf("[Tende] Command %s on %d\n",
                                cmdName, stato.activeTenda);

                // Feedback visivo (flash)
                NexManager::sendFormatted("click q%d,1", stato.activeTenda);
                delay(80);
                NexManager::sendFormatted("click q%d,0", stato.activeTenda);

                // Invia comando MQTT
                Tende target[] = {(Tende)stato.activeTenda};
                mqttWifi::sendTende(target, 1, cmd, 0);
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