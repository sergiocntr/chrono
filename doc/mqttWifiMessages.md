# ✉️ Gestione Messaggi MQTT
[← Torna al README](../README.md)

Questo modulo gestisce il parsing dei messaggi in ingresso e la formattazione dei dati in uscita, agendo da ponte tra il cloud/HUB e l'interfaccia fisica.

## Callback di Ricezione (callback)

```mermaid
graph TD
    C_Start[Messaggio Ricevuto] --> C_Topic{Analisi Topic}
    
    C_Topic -- systemTopic --> C_Sys[Gestione Sleep/Update]
    C_Topic -- acquaTopic --> C_Acq[Update stato.relays e Nex icon]
    C_Topic -- tendeTuyaState --> C_Tende[Update posizioni tende BIN]
    
    C_Topic -- Altro --> C_JSON{Parsing JSON}
    C_JSON -- Successo --> C_Disp{Inner Topic?}
    
    C_Disp -- UpTime --> C_Time[Aggiorna Ora/Giorno su Display]
    C_Disp -- DHTCamera --> C_Cam[Aggiorna Temp/Hum Interna]
    C_Disp -- Caldaia --> C_Boiler[Aggiorna Temp Acqua]
    C_Disp -- EneMain --> C_Ene[Aggiorna Watt Energia]
```

## Invio Posizioni Tende (pubTende)

```mermaid
graph TD
    P_Start([Start]) --> P_Mask{Maschera Selezione?}
    P_Mask -- 0 --> P_End([Fine])
    P_Mask -- >0 --> P_Loop[Per ogni bit impostato]
    
    P_Loop --> P_Doc[Crea JSON con ID Tenda e Comando]
    P_Doc --> P_Pub[mqttWifi::publish]
    P_Pub --> P_End
```
