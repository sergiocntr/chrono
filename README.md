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
#### 2.1.0  
- Chrono versione = 148;
- Creazione README
- Integrazione tende con modulo Tuya
- **Viene mantenuto ArduinoJson v5** perche' gestisce meglio l'interfaccia Nextion
- **Abilitati pulsanti Up e Down** : apre persiane a 80% e chiude tutte le persiane e tende.