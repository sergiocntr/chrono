## Read me
**Work in progress**
### Scopo del progetto

- la creazione di una interfaccia Touch per il controllo del riscaldamento e delle tende di casa
### Logica del dispositivo
Il dispositivo segue questa logica:
1. Setup WiFi :void
    - Si occupa del setup del modulo ESP cone le credenziali  al WiFi di casa
2. Setup MQTT :void
    - Si occupa della integrazione con il MQTT ,credenziali ,nome modulo ,callback
3. Connect WiFi :bool
    - Si occupa della connessione al WiFi e MQTT di casa
    - 0 connessione riuscita , 1 connessione fallita
4. ......

### Versioni - Change log
#### 3.0.0  

### ✨ Nuova Architettura Modulare

#### 🎯 **Principali Cambiamenti**

1. **Rimossa dipendenza dalla libreria Nextion ufficiale**
   - Niente più classi pesanti (NexText, NexButton, NexCrop)
   - Comunicazione diretta con comandi Nextion via seriale
   - Riduzione drastica del footprint di memoria

2. **Nuova funzione universale `sendFormatted()`**
   ```cpp
   // Unica funzione per TUTTI i comandi Nextion
   NexManager::sendFormatted("%s.txt=\"%s\"", "t0", "testo");     // Testo
   NexManager::sendFormatted("%s.val=%d", "h0", 50);              // Valore
   NexManager::sendFormatted("%s.pic=%d", "c0", 5);               // Immagine
   NexManager::sendFormatted("vis %s,%d", "q0", 1);               // Visibilità
   NexManager::sendFormatted("page %d", 1);                       // Pagina
#### 2.1.0  
- Chrono versione = 148;
- Creazione README
- Integrazione tende con modulo Tuya
- **Viene mantenuto ArduinoJson v5** perche' gestisce meglio l'interfaccia Nextion
- **Abilitati pulsanti Up e Down** : apre persiane a 80% e chiude tutte le persiane e tende.