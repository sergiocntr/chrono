# NexManager: Gestione Display

Il modulo `NexManager` si occupa di tutta la comunicazione seriale con il display Nextion, astraendo l'invio di comandi e il parsing dei dati in arrivo.

## Parsing degli Eventi (poll)

```mermaid
graph TD
    P_Start([Start poll]) --> P_Avail{Dati Seriali?}
    P_Avail -- No --> P_EndEmpty([Ritorna Evento nullo])
    P_Avail -- Sì --> P_Peek[Peek Header byte]
    
    P_Peek --> P_Header{Header?}
    P_Header -- 0x65 --> P_Touch[Leggi P, ID, E + 3 byte FF]
    P_Header -- 0x66 --> P_Page[Leggi Page ID + 3 byte FF]
    P_Header -- 0x71 --> P_Num[Leggi Valore 4 byte + 3 byte FF]
    P_Header -- Altro --> P_Discard[Scarta byte]
    
    P_Touch --> P_Valid[Ritorna Evento Valido]
    P_Page --> P_Valid
    P_Num --> P_EndEmpty
    P_Discard --> P_Start
```

## Aggiornamento Grafica (refreshCurrentPage)

```mermaid
graph TD
    R_Start([Inizio Refresh]) --> R_Page{Pagina Corrente?}
    R_Page -- 0 --> R_Home[Invia Ora, Day, Temp, Hum, KWh]
    R_Page -- 1 --> R_Reset[resetTendeTimer]
    R_Reset --> R_Tende[Invia Stati e Visibilità Slider]
    
    R_Home --> R_Format[Formattazione stringhe dtostrf]
    R_Format --> R_Send[NexManager::sendFormatted]
    R_Send --> R_End([Fine])
    R_Tende --> R_Send
```
