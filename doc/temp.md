# Gestione Temperatura: temp.cpp

Gestisce la lettura locale del sensore DHT22.

## Logica di Lettura (getLocalTemp)

```mermaid
graph TD
    T_Start([Chiamata getLocalTemp]) --> T_Read[Lettura dht.getTempAndHumidity]
    T_Read --> T_Acc[Accumula valori per media]
    T_Acc --> T_Count{Contatore == 4?}
    
    T_Count -- No --> T_End([Fine])
    T_Count -- Sì --> T_Comfort[Calcola Comfort Ratio]
    T_Comfort --> T_Send[mqttWifi::sendData]
    T_Send --> T_Reset[Resetta contatore e accumulatori]
```

## Invio Dati (sendData)
Avviene in `mqttWifiMessages.cpp` ma è chiamata da `temp.cpp`.

```mermaid
graph TD
    S_Start[Inizio sendData] --> S_Doc[Crea JSON con Media T/H e Comfort]
    S_Doc --> S_Pub[mqttWifi::publish casaSensTopic]
    S_Pub --> S_End([Fine])
```
