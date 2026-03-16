# 🔄 Main Loop & Inizializzazione
[← Torna al README](../README.md)

Il file `main.cpp` è il cuore dell'applicazione. Gestisce il setup del sistema e il ciclo di esecuzione principale garantendo reattività tramite task asincroni.

## Diagramma di Flusso Principale

```mermaid
graph TD
    subgraph "Inizializzazione (setup)"
    S_Start([Start]) --> S_Delay[Delay 3s]
    S_Delay --> S_Nex[NexManager::begin]
    S_Nex --> S_Wifi[mqttWifi::setupCompleto]
    S_Wifi --> S_DHT[tempDHT::setupTemp]
    S_DHT --> S_Wake[NexManager::wakeupNextion]
    S_Wake --> S_End([End Setup])
    end

    subgraph "Loop Corrente (loop)"
    L_Start([Inizio Loop]) --> L_CheckT{Tempo Lettura Sensori?}
    L_CheckT -- Sì --> L_Read[tempDHT::getLocalTemp]
    L_CheckT -- No --> L_Conn[mqttWifi::gestisciConnessione]
    L_Read --> L_Conn
    L_Conn --> L_SD[smartDelay 10000ms]
    L_SD --> L_Start
    end
```

## Funzione smartDelay
Questa funzione è critica poiché permette l'esecuzione di task in background (MQTT e Touch) durante le attese.

```mermaid
graph TD
    SD_Start(Entrata smartDelay) --> SD_While{Tempo trascorso < richiesto?}
    SD_While -- Sì --> SD_MQTT[mqttWifi::client.loop]
    SD_MQTT --> SD_Poll[NexManager::poll]
    SD_Poll --> SD_Valid{Evento Touch?}
    SD_Valid -- Sì --> SD_Case{Quale Pagina?}
    SD_Case -- 0 --> SD_P0[handleHomePage]
    SD_Case -- 1 --> SD_P1[handleTendePage]
    SD_Valid -- No --> SD_TO[In Pagina 1?]
    SD_TO -- Sì --> SD_CheckTO[checkTendeTimeout]
    SD_CheckTO --> SD_Wait[delay 10ms]
    SD_TO -- No --> SD_Wait
    SD_Wait --> SD_While
    SD_While -- No --> SD_End(Fine smartDelay)
```
