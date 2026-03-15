#pragma once
#include "NexManager.h"
#include <ArduinoJson.h>

// Handler per pagina Home (ID 0)
void handleHomePage(const NexManager::TouchEvent& evt);

// Handler per pagina Tende (ID 1)
void handleTendePage(const NexManager::TouchEvent& evt);

// Controlla se il timeout di inattività (15s) sulla pagina Tende è scaduto.
// Da chiamare ad ogni ciclo di smartDelay quando currPage == 1.
void checkTendeTimeout();

// Resetta il timer di inattività della pagina Tende (es. all'ingresso nella pagina).
void resetTendeTimer();
// Aggiungi qui nuovi handler per altre pagine
// void handleSettingsPage(const NexManager::TouchEvent& evt);
// void handleStatsPage(const NexManager::TouchEvent& evt);