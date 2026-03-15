# Documentazione Progetto Chrono

In questo documento è descritta l'architettura del firmware e il flusso logico tra i principali moduli.

## 1. Visione d'Insieme (Main Loop)
Il file `main.cpp` gestisce il ciclo di vita principale. Utilizza la funzione `smartDelay` per garantire che il sistema non si blocchi mai durante le attese, permettendo al client MQTT di elaborare i messaggi e al display di rispondere ai tocchi.

```mermaid
graph TD
    subgraph "Main Loop (main.cpp)"
    Start([Avvio]) --> Setup[setup]
    Setup --> Loop[loop]
    
    Loop --> ReadTemp{Tempo lettura sensori?}
    ReadTemp -- Sì --> getLocalTemp[tempDHT::getLocalTemp]
    ReadTemp -- No --> ConnCheck[mqttWifi::gestisciConnessione]
    
    ConnCheck --> SmartDelay[smartDelay 10s]
    
    subgraph "SmartDelay (Gestione Eventi)"
        SD_Start(Ciclo 10s) --> MQTT_Loop[mqtt::client.loop]
        MQTT_Loop --> Nex_Poll[NexManager::poll]
        Nex_Poll --> EventValid{Evento Valido?}
        EventValid -- Sì --> Dispatch[Dispatch Pagina Corrente]
        Dispatch --> Case0[Pagina 0: handleHomePage]
        Dispatch --> Case1[Pagina 1: handleTendePage]
        EventValid -- No --> CheckTimeout[In Pagina 1?]
        CheckTimeout -- Sì --> TendeTimeout[checkTendeTimeout]
        CheckTimeout -- No --> Delay10[delay 10ms]
        Delay10 --> SD_End(Fine Ciclo)
    end
    
    SD_End --> Loop
    end
```

## 2. Gestione Display (NexManager)
`NexManager.cpp` astrae la comunicazione seriale con il pannello Nextion. Trasforma i byte grezzi in eventi strutturati.

```mermaid
graph TD
    subgraph "Gestione Display (NexManager.cpp)"
    Poll[NexManager::poll] --> Available{Serial Available?}
    Available -- No --> ReturnEmpty[Ritorna Evento Vuoto]
    Available -- Sì --> Peek[Peek Header]
    
    Peek --> HeaderType{Tipo?}
    HeaderType -- 0x65 --> Touch[Evento Touch: Leggi Pagina/Comp/Tipo]
    HeaderType -- 0x66 --> Page[Invio Pagina: Leggi nuovo ID]
    HeaderType -- 0x71 --> Numeric[Risposta Numerica]
    
    Touch --> Valid[Ritorna Evento Valido]
    Page --> Valid
    ReturnEmpty --> Out([Fine])
    Valid --> Out
    end
```

## 3. Logica delle Pagine (PageHandlers)
`PageHandlers.cpp` contiene la logica di business associata agli elementi grafici.

```mermaid
graph TD
    subgraph "Logica Pagine (PageHandlers.cpp)"
    HandleP0[handleHomePage] --> CompID{ID Componente?}
    CompID -- 5 --> PubAcqua[Inverti Rele' Acqua]
    CompID -- 10 --> PubRisc[Inverti Rele' Riscaldamento]
    CompID -- 7/8 --> PubT[pubTende: APRI/CHIUDI TUTTE]
    CompID -- 13 --> SwitchP1[NexManager::setPage 1]
    
    HandleP1[handleTendePage] --> TCompID{ID Componente?}
    TCompID -- 10-14 --> Select[Aggiorna Maschera Selezione]
    TCompID -- 15-17 --> Move[pubTende: STOP/UP/DOWN]
    
    PubAcqua --> MQTT_Pub[mqtt::publish]
    PubT --> MQTT_Pub
    Move --> MQTT_Pub
    end
```

## 4. Ricezione Dati e Callback (mqttWifiMessages)
Questo modulo gestisce l'arrivo dei dati dall'esterno (MQTT) e l'aggiornamento dello stato globale e della grafica.

```mermaid
graph TD
    subgraph "Comunicazione (mqttWifiMessages.cpp)"
    Callback(Callback MQTT) --> TopicType{Quale Topic?}
    
    TopicType -- System --> SysCmd[Sleep o Update]
    TopicType -- Acqua/Risc --> FeedbackR[Aggiorna Icone Rele']
    TopicType -- TendeState --> FeedbackT[Aggiorna Slider Tende]
    TopicType -- Sensors --> Parsing[Parsing JSON: Orario, Temp Ext, Energia]
    
    FeedbackR --> NexUpdate[NexManager::sendFormatted]
    FeedbackT --> NexUpdate
    Parsing --> StateUpdate[Aggiorna Struttura 'stato']
    StateUpdate --> NexUpdate
    end
```

## 5. Lettura Sensore Locale (temp.cpp)
Implementa la lettura del DHT22 con una logica di media mobile per maggiore stabilità.

```mermaid
graph TD
    subgraph "Sensore Locale (temp.cpp)"
    Read[getLocalTemp] --> Status{Lettura OK?}
    Status -- No --> LogErr[Pubblica Errore]
    Status -- Sì --> Accumulate[Accumula per Media]
    
    Accumulate --> CheckCount{4 letture fatte?}
    CheckCount -- Sì --> Comfort[Calcola Comfort Ratio]
    Comfort --> Send[mqttWifi::sendData]
    Send --> Reset[Resetta Accumulatori]
    CheckCount -- No --> End([Fine])
    end
```
