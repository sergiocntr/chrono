# Changelog Gestione Tende/Persiane - Chrono2

In questa sessione abbiamo aggiornato il sistema di comunicazione MQTT per le tende/persiane per renderlo compatibile con il nuovo bridge Node-Red e il feedback diretto da Tuya.

## 1. Modifica Protocollo di Invio (Chrono -> MQTT)
La funzione `pubTende` è stata interamente riscritta per passare da un singolo messaggio con array di tende a **messaggi individuali** per ogni dispositivo.

- **Topic:** Cambiato da `tendeTuya` a `tendeTuyaCmd`.
- **Payload:** Ridotto e semplificato per il parsing rapido in Node-Red.
  - Formato: `{"t": ID, "c": Comando, "p": Percentuale}`
  - Le chiavi sono state abbreviate: `t` (tenda), `c` (comando), `p` (percentuale).
- **Controllo Flusso:** Inserito un `delay(50)` tra gli invii multipli (es. comandi "T_CLOSE Tutto") per garantire la stabilità della connessione WiFi/MQTT dell'ESP.

## 2. Gestione Feedback di Stato (MQTT -> Chrono)
Implementata la logica per processare i messaggi di ritorno (feedback) provenienti dal topic `tendeTuyaState`.

- **Parsing JSON:** Aggiornata la `callback` in `mqttWifiMessages.cpp` per riconoscere il feedback Tuya.
- **Struttura Feedback:** `{"topic":"tende", "t": ID, "p": Percentuale}`.
- **Sincronizzazione UI:**
  - Lo stato ricevuto viene salvato nell'array `stato.pos[ID]`.
  - Se l'utente si trova sulla pagina Tende (Page 1), gli slider del Nextion (`ts_bar`, `tl_bar`, `ps_bar`, `pl_bar`, `pc_bar`) vengono aggiornati in tempo reale con il valore della percentuale.

## 3. Refactoring Nomenclatura e Costanti
- **Ridenominazione:** Aggiornato l'enum `Tende` in `impostazioni.h` per riflettere la natura dei dispositivi:
  - `TAPPA_SALOTTO` -> `PERSIANA_SALOTTO`
  - `TAPPA_LEO` -> `PERSIANA_LEO`
  - `TAPPA_CAMERA` -> `PERSIANA_CAMERA`
- **Correzione Enum:** Elinato il comando `PARZIALE` e `TENDE_STATUS`.
- **Debug:** Elevato il `DEBUG_LEVEL` a **3 (Verbose)** per monitorare correttamente i feedback MQTT durante i test.

## 4. File Coinvolti
- `src_nextion/PageHandlers.cpp`: Aggiornamento logica `pubTende`.
- `src_crono/PageHandlers.cpp`: Aggiornamento logica `pubTende` e nomi costanti.
- `src_crono/mqttWifiMessages.cpp`: Implementazione ricezione feedback e aggiornamento slider Nextion.
- `lib/impostazioni/impostazioni.h`: Modifica enum e livelli di debug.
