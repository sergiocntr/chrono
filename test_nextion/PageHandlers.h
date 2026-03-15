#pragma once
#include "NexManager.h"
#include <ArduinoJson.h>

// Handler per pagina Home (ID 0)
void handleHomePage(const NexManager::TouchEvent& evt);

// Handler per pagina Tende (ID 1)
void handleTendePage(const NexManager::TouchEvent& evt);

// Aggiungi qui nuovi handler per altre pagine
// void handleSettingsPage(const NexManager::TouchEvent& evt);
// void handleStatsPage(const NexManager::TouchEvent& evt);