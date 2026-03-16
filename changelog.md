# 📜 Registro delle Modifiche (Changelog)

Tutte le modifiche significative al progetto **Chrono 2** verranno documentate in questo file.

---

## [4.0.0] - 2026-03 ✨ Nuova Architettura Modulare

Questa versione rappresenta una pietra miliare nello sviluppo del progetto. Il firmware è stato riscritto per massimizzare le performance e la manutenibilità.

### 🎯 Principali Innovazioni
- **Indipendenza dal Driver Nextion**: Rimossa la dipendenza dalle classi pesanti (`NexText`, `NexButton`, ecc.). Ora la comunicazione avviene tramite comandi seriali diretti, riducendo drasticamente l'occupazione di memoria.
- **Funzione `sendFormatted()`**: Introdotto un sistema universale per l'invio di comandi al display:
  ```cpp
  NexManager::sendFormatted("%s.txt=\"%s\"", "t0", "testo");
  ```
- **Gestione UX**: Centralizzato il timeout di inattività (15s) per il ritorno automatico alla Home.
- **Logica Tende**: Supporto per messaggi MQTT individuali per dispositivo, migliorando la compatibilità con Node-RED e Tuya.

### 📦 Ottimizzazione Risorse
Abbiamo ottenuto una riduzione del firmware da **1.3 MB a 327 KB** (**-76%**).

| Metrica | Prima | Dopo | Miglioramento |
| :--- | :--- | :--- | :--- |
| **Flash** | 1372 KB | 328 KB | **-76%** |
| **RAM** | ~50 KB | ~15 KB | **-70%** |
| **Payload MQTT** | JSON (80 byte) | Binario (3 byte) | **-96%** |
| **Stabilità** | Fragile | Robusta | ⬆️⬆️⬆️ |

---

## [2.1.0] - 2026-02
- **Integrazione Tuya**: Piena compatibilità con il modulo tende Tuya.
- **UX**: Abilitati pulsanti fisici/touch Up/Down per aperture rapide (80%).
- **ArduinoJson**: Mantenuta v5 per compatibilità specifica (aggiornata poi a v6 nella 4.0.0).

---

## [Archivio Modifiche Tende]
### Modifica Protocollo (Chrono -> MQTT)
- **Topic**: `tendeTuya` ➡️ `tendeTuyaCmd`
- **Payload**: Abbreviato (`t`, `c`, `p`) per efficienza energetica e velocità di trasmissione.
- **Delay**: Inserito `delay(50)` tra invii multipli per evitare saturazione del buffer WiFi dell'ESP.

### Feedback di Stato (MQTT -> Chrono)
- **Topic**: `tendeTuyaState`
- **Sincronizzazione**: Aggiornamento degli slider in tempo reale sulla pagina 1 senza ricaricare l'intera interfaccia ("scivolata").

---

> *"La semplicità è la massima sofisticazione."*