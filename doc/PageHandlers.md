# 🖱️ PageHandlers: Logica Interfaccia
[← Torna al README](../README.md)

Questo modulo contiene la logica di business che risponde agli eventi touch. Separa la gestione degli eventi dalla comunicazione di basso livello.

## Gestione Pagina Home (handleHomePage)

```mermaid
graph TD
    H_Evt[Evento Ricevuto] --> H_Comp{Componente ID?}
    H_Comp -- 5 (Water) --> H_Acqua[MQTT: Cambia stato Acqua]
    H_Comp -- 10 (Heat) --> H_Risc[MQTT: Cambia stato Riscaldamento]
    H_Comp -- 7/8 (Curtains) --> H_Tende[MQTT: Comando T_OPEN/T_CLOSE]
    H_Comp -- 13 (Setup) --> H_Page[NexManager::setPage 1]
```

## Gestione Pagina Tende (handleTendePage)

```mermaid
graph TD
    T_Evt[Evento Ricevuto] --> T_Reset[Resetta Timer Inattività]
    T_Reset --> T_Comp{Componente ID?}
    
    T_Comp -- 10-14 (Crops) --> T_Select[Aggiungi a Maschera Selezione]
    T_Select --> T_Vis[Aggiorna visibilità slider correlati]
    
    T_Comp -- 15-17 (Buttons) --> T_Check{Qualcosa selezionato?}
    T_Check -- No --> T_Ignore[Ignora]
    T_Check -- Sì --> T_Move[mqttWifi::pubTende]
```

## Timeout Tende (checkTendeTimeout)

```mermaid
graph TD
    TO_Start([checkTendeTimeout]) --> TO_Check{Inattività > 15s?}
    TO_Check -- Sì --> TO_Home[NexManager::setPage 0]
    TO_Check -- No --> TO_End([Fine])
    
    subgraph "Sorgenti di Reset del Timer"
    R_Entry[Ingresso Pagina 1] -.-> R_Func[resetTendeTimer]
    R_Touch[Qualsiasi Touch P1] -.-> R_Func
    end
```
