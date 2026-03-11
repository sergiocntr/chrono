#include "NexManager.h"
// #include <EspNowManager.h>
#include <impostazioni.h>

namespace mqttWifi
{
  void sendTende(Tende tendeTargets[], size_t numTende, ComandoTende comando,
                 int percentuale);
  bool publish(const char *topic, const char *message, bool retained = false);
} // namespace mqttWifi

#if defined(ESP32_BUILD)
#define NEX_SERIAL Serial1
#else
#define NEX_SERIAL Serial
#endif

namespace NexManager
{
  static uint32_t lastRefresh = 0;

  void begin(unsigned long baud)
  {
#if defined(ESP32_BUILD)
    NEX_SERIAL.begin(baud, SERIAL_8N1, NEXTION_RX, NEXTION_TX);
#else
    NEX_SERIAL.begin(baud);
#endif
    delay(100);
    sendCommand("");
    logSerialPrintf("[NexManager] Initialized at %lu baud\n", baud);
  }
  void shutdownNextion()
  {
    delay(100);
    sendCommand("thup=1");
    sendCommand("sleep=1");
    delay(100);
  }
  void wakeupNextion()
  {
    sendCommand("dim=20");
    sendCommand("sleep=0");
    delay(100);
  }
  void sendCommand(const char *cmd)
  {
    NEX_SERIAL.print(cmd);
    NEX_SERIAL.write(0xFF);
    NEX_SERIAL.write(0xFF);
    NEX_SERIAL.write(0xFF);
    delay(5);
  }

  void setText(const char *obj, const char *text)
  {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s.txt=\"%s\"", obj, text);
    sendCommand(buffer);
  }

  void setValue(const char *obj, uint32_t value)
  {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s.val=%u", obj, value);
    sendCommand(buffer);
  }

  void setPage(const char *pageId)
  {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "page %s", pageId);
    sendCommand(buffer);
  }

  // Helper per valori float
  void setFloat(const char *obj, float value, int decimals = 1)
  {
    char buffer[32];
    dtostrf(value, 4, decimals, buffer);
    setText(obj, buffer);
  }

  // Comando generico con formattazione
  void sendFormatted(const char *format, ...)
  {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    sendCommand(buffer);
  }

  TouchEvent poll()
  {
    TouchEvent evt; // Inizializzato con isValid=false

    if (NEX_SERIAL.available() < 7)
      return evt;

    if (NEX_SERIAL.peek() != 0x65 && NEX_SERIAL.peek() != 0x71 &&
        NEX_SERIAL.peek() != 0x70)
    {
      NEX_SERIAL.read();
      return evt;
    }

    uint8_t header = NEX_SERIAL.read();

    switch (header)
    {
    case 0x65:
      evt.isValid = true;
      evt.page = NEX_SERIAL.read();
      evt.component = NEX_SERIAL.read();
      evt.event = NEX_SERIAL.read(); // 0=release, 1=press

      // Consuma i 3 byte di terminazione
      for (int i = 0; i < 3; i++)
        NEX_SERIAL.read();

      logSerialPrintf("[NexTouch] P:%u ID:%u E:%u\n",
                      evt.page, evt.component, evt.event);
      break;

    case 0x66:
      evt.isValid = true;
      evt.page = NEX_SERIAL.read();

      for (int i = 0; i < 3; i++)
        NEX_SERIAL.read();

      logSerialPrintf("[NexPage] %u\n", evt.page);
      break;

    case 0x70:
      // String
      // skip for now
      while (NEX_SERIAL.available() && NEX_SERIAL.peek() != 0xFF)
        NEX_SERIAL.read();
      for (int i = 0; i < 3; i++)
        if (NEX_SERIAL.available())
          NEX_SERIAL.read();
      break;

    case 0x71:
      // Numeric
      uint32_t val = 0;
      for (int i = 0; i < 4; i++)
        val |= ((uint32_t)NEX_SERIAL.read() << (8 * i));
      for (int i = 0; i < 3; i++)
        NEX_SERIAL.read();
      logSerialPrintf("[NexNum] %u\n", val);
      break;

      // default:

      //  break;
    }

    // if (header == 0x66)
    // {
    //   evt.isValid = true;
    //   evt.page = NEX_SERIAL.read();

    //   for (int i = 0; i < 3; i++)
    //     NEX_SERIAL.read();

    //   logSerialPrintf("[NexPage] %u\n", evt.page);
    // }

    // else if (header == 0x65)
    // { // Touch Event
    //   evt.isValid = true;
    //   evt.page = NEX_SERIAL.read();
    //   evt.component = NEX_SERIAL.read();
    //   evt.event = NEX_SERIAL.read(); // 0=release, 1=press

    //   // Consuma i 3 byte di terminazione
    //   for (int i = 0; i < 3; i++)
    //     NEX_SERIAL.read();

    //   logSerialPrintf("[NexTouch] P:%u ID:%u E:%u\n",
    //                   evt.page, evt.component, evt.event);
    // }
    // else if (header == 0x71)
    // { // Numeric
    //   uint32_t val = 0;
    //   for (int i = 0; i < 4; i++)
    //     val |= ((uint32_t)NEX_SERIAL.read() << (8 * i));
    //   for (int i = 0; i < 3; i++)
    //     NEX_SERIAL.read();
    //   logSerialPrintf("[NexNum] %u\n", val);
    // }
    // else if (header == 0x70)
    // { // String
    //   // skip for now
    //   while (NEX_SERIAL.available() && NEX_SERIAL.peek() != 0xFF)
    //     NEX_SERIAL.read();
    //   for (int i = 0; i < 3; i++)
    //     if (NEX_SERIAL.available())
    //       NEX_SERIAL.read();
    // }

    return evt;
  }

  void refreshCurrentPage()
  {
    uint32_t now = millis();
    if (now - lastRefresh < 5000)
      return;
    lastRefresh = now;

    char buff[16];

    if (stato.currPage == 0)
    {
      //logSerialPrintf("[refreshCurrentPage] Page %d", stato.currPage);
      // Raggruppa operazioni simili
      const char *textWidgets[] = {"Ncurr_hour", "Nday", "Ntcurr", "Nout_temp",
                                   "Nout_hum", "Nin_hum", "Nwater_temp", "Nset_temp"};
      const char *cropWidgets[] = {"Nrisc_on", "Nwater_on", "Nalarm"};
      const char *values[] = {stato.timeStr, stato.dayStr, "", "", "", "", "", ""};

      // Aggiorna testo data/ora
      NexManager::sendFormatted("Ncurr_hour.txt=\"%s\"", stato.timeStr);
      NexManager::sendFormatted("Nday.txt=\"%s\"", stato.dayStr);

      // Aggiorna valori float
      dtostrf(stato.temps[INT], 4, 1, buff);
      NexManager::sendFormatted("Ntcurr.txt=\"%s\"", buff);

      dtostrf(stato.temps[EXT], 4, 1, buff);
      NexManager::sendFormatted("Nout_temp.txt=\"%s\"", buff);

      dtostrf(stato.hums[EXT], 4, 1, buff);
      NexManager::sendFormatted("Nout_hum.txt=\"%s\"", buff);

      dtostrf(stato.hums[INT], 4, 1, buff);
      NexManager::sendFormatted("Nin_hum.txt=\"%s\"", buff);

      dtostrf(stato.waterTemp, 4, 1, buff);
      NexManager::sendFormatted("Nwater_temp.txt=\"%s\"", buff);

      // Potenza come intero
      NexManager::sendFormatted("Nset_temp.txt=\"%d\"", stato.powerW);

      NexManager::sendFormatted("Ncurr_hour.txt=\"TEST\""); // <--------- !!!!!!!!!! Da togliere
    }
    else if (stato.currPage == 1)
    {
      const char *sliderNames[] = {"pl_bar", "tl_bar", "ps_bar", "ts_bar", "pc_bar"};

      for (int i = 0; i < 5; i++)
      {
        // Una sola chiamata per gestire visibilità e valore
        int visibilità = (i == stato.activeTenda) ? 1 : 0;
        NexManager::sendFormatted("vis %s,%d", sliderNames[i], visibilità);

        if (visibilità)
        {
          NexManager::sendFormatted("%s.val=%d", sliderNames[i], stato.pos[i]);
        }
      }
    }
  }

} // namespace NexManager
