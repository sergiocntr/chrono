#pragma once
#include <Arduino.h>
// #include <Nextion.h>

#define DEBUG_CHRONO
#define DEBUG_UDP_LOG // UDP sempre attivo
#define DEBUG_LEVEL 1 // 0=solo errori, 1=warning, 2=info, 3=verbose

#define UDP_LOG_IP "192.168.1.100" // broadcast, funziona senza IP fisso
#define UDP_LOG_PORT 4444

// ============================================================
//  Implementazione UDP logger
// ============================================================
#ifdef DEBUG_UDP_LOG
#include <WiFiUdp.h>
extern WiFiUDP udpLog; // definito nel .cpp principale

void udpLogBegin();
void udpLogSend(const char *msg);
void udpLogSend_f(const char* fmt, ...);

// Helper per convertire qualsiasi tipo, incluso IPAddress
template <typename T>
inline String _toStr(T val) { return String(val); }
inline String _toStr(IPAddress ip) { return ip.toString(); }

#define LOG_ERROR(...) udpLogSend_f(__VA_ARGS__) // sempre attivo

#ifdef DEBUG_UDP_LOG
#if DEBUG_LEVEL >= 1
#define LOG_WARN(...) udpLogSend_f(__VA_ARGS__)
#else
#define LOG_WARN(...) \
  do                  \
  {                   \
  } while (0)
#endif

#if DEBUG_LEVEL >= 2
#define LOG_INFO(...) udpLogSend_f(__VA_ARGS__)
#else
#define LOG_INFO(...) \
  do                  \
  {                   \
  } while (0)
#endif

#if DEBUG_LEVEL >= 3
#define LOG_VERBOSE(...) udpLogSend_f(__VA_ARGS__)
#else
#define LOG_VERBOSE(...) \
  do                     \
  {                      \
  } while (0)
#endif
#endif
/*
#define logSerialPrint(a)        do { \
                                   udpLogSend(_toStr(a).c_str()); \
                                 } while(0)
#define LOG_VERBOSE(a)      do { \
                                   String _s = _toStr(a) + "\n"; \
                                   udpLogSend(_s.c_str()); \
                                 } while(0)
#define LOG_VERBOSE(...)     do { \
                                   char _buf[192]; \
                                   snprintf(_buf, sizeof(_buf), __VA_ARGS__); \
                                   udpLogSend(_buf); \
                                 } while(0)
*/
#elif defined(DEBUG_CHRONO)
// Seriale classica — invariata
#define logSerial Serial
#define logSerialBegin(a) logSerial.begin(a)
#define logSerialPrint(a) logSerial.print(a)
#define LOG_VERBOSE(a) logSerial.println(a)
#define LOG_VERBOSE(...) logSerial.printf(__VA_ARGS__)

#else
// Produzione — tutto sparisce
#define logSerialBegin(a) \
  do                      \
  {                       \
  } while (0)
#define logSerialPrint(a) \
  do                      \
  {                       \
  } while (0)
#define LOG_VERBOSE(a) \
  do                        \
  {                         \
  } while (0)
#define LOG_VERBOSE(...) \
  do                         \
  {                          \
  } while (0)
#endif

extern const char *mqttId;
struct tempStr
{
  float t;
  float h;
  uint8_t confort;
};
extern tempStr myTemp;

enum Tende
{
  TENDA_SALOTTO, // 0
  TENDA_LEO,     // 1
  TAPPA_SALOTTO, // 2
  TAPPA_LEO,     // 3
  TAPPA_CAMERA   // 4
};

// Enum per i comandi delle tende
enum ComandoTende
{
  CHIUDI,      // 0
  APRI,        // 1
  FERMA,       // 2 (STOP)
  PARZIALE,    // 3
  TENDE_STATUS // 4
};
extern ComandoTende comandoTenda;

// ========== ENUM MOTIVI SPEGNIMENTO ==========
enum MotivoSpegnimento
{
  PUBLISH_FALLITO = 0,
  COMANDO_SYSTEM_TOPIC = 1,
  WIFI_TIMEOUT_CONNESSIONE = 2,
  MQTT_TIMEOUT_CONNESSIONE = 3,
  WIFI_FALLITO_SETUP = 4,
  MQTT_FALLITO_RISVEGLIO = 5,
  WIFI_FALLITO_RISVEGLIO = 6,
  NEXTION_SETUP_FAILED = 7,
  DHT_SETUP_FAILED = 8,
  SETUP_OK = 253,
  CONN_OK = 254,
  SHUTDOWN_FROM_MQTT = 255
};
extern MotivoSpegnimento m_wifi_status;
// extern NexTouch *nex_listen_list[];
// extern uint8_t db_array_value[4];
// extern NexText Nset_temp;
// extern NexText Ntcurr;
// extern NexText Nout_temp;
// extern NexCrop Nwater_on;
// extern NexText Nout_hum;
// extern NexText Nin_hum;
// extern NexText Ncurr_hour;
// extern NexText Nwater_temp;
// extern NexText Nday;
// extern NexButton Nb_up;
// extern NexButton Nb_down;
// extern NexCrop Nrisc_on;
// extern NexCrop Nalarm;
// ========== INDICI PER GLI ARRAY DI STATO ==========
enum SensIdx
{
  INT = 0, // Interno (Chrono)
  EXT = 1, // Esterno (ESPmeteo)
  BAG = 2, // Bagno
  MAX_SENS = 4
};

enum RelayIdx
{
  RISCALDAMENTO = 0,
  ACQUA = 1,
  ALLARME = 2,
  CAMERA = 3,
  TERRAZZA = 4,
  CALDAIA = 5,
  ENERGIA = 6,
  MAX_RELAY = 7
};
struct __attribute__((packed)) SystemState
{
  float temps[MAX_SENS];
  float hums[MAX_SENS];
  float waterTemp;
  uint16_t powerW;
  uint8_t pos[6]; // slider tende
  bool relays[MAX_RELAY];
  uint8_t currPage;
  int8_t activeTenda;
  uint32_t lastUpdate;
  char timeStr[16]; // Esempio: "HH:MM"
  char dayStr[32];  // Esempio: "Lunedi"
};

extern SystemState stato;
