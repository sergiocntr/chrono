# 🌐 Connettività WiFi & MQTT
[← Torna al README](../README.md)

Gestisce la resilienza della connessione di rete e il protocollo di comunicazione con il broker. Implementa logiche di riconnessione automatica e queuing dei messaggi.

## Gestione Connessioni (gestisciConnessione)

```mermaid
graph TD
    G_Start([Inizio]) --> G_WifiCheck{WiFi Connesso?}
    
    G_WifiCheck -- No --> G_WifiRetry{Tentativi < 3?}
    G_WifiRetry -- Sì --> G_WifiConn[Tenta Connessione WiFi]
    G_WifiRetry -- No --> G_WifiFail[Ritorna Errore Timeout WiFi]
    
    G_WifiCheck -- Sì --> G_MQTTCheck{MQTT Connesso?}
    G_WifiConn --> G_MQTTCheck
    
    G_MQTTCheck -- No --> G_MQTTRetry{Tentativi < 3?}
    G_MQTTRetry -- Sì --> G_MQTTConn[Tenta Connessione MQTT]
    G_MQTTRetry -- No --> G_MQTTFail[Ritorna Errore Timeout MQTT]
    
    G_MQTTConn --> G_Sub[Sottoscrizione Topic]
    G_Sub --> G_OK[Ritorna CONN_OK]
    
    G_MQTTCheck -- Sì --> G_Loop[client.loop]
    G_Loop --> G_OK
```

## Pubblicazione con Retry (publish)

```mermaid
graph TD
    P_Start([Start Publish]) --> P_Check{Client Connesso?}
    P_Check -- No --> P_Error[Ritorna False]
    
    P_Check -- Sì --> P_Loop[Tentativi: 3]
    P_Loop --> P_Try[client.publish]
    P_Try -- Successo --> P_OK[Ritorna True]
    P_Try -- Fallito --> P_Wait[Delay 50ms]
    P_Wait --> P_Loop
    
    P_Loop -- Esauriti --> P_Sleep[adessoDormo: PUBLISH_FALLITO]
```
