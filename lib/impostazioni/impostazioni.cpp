#include "impostazioni.h"
// #include "Nextion.h"
tempStr myTemp;
ComandoTende comandoTenda;
#ifdef ESP8266_BUILD
const char *mqttId = "Chrono";

#elif ESP32_BUILD
const char *mqttId = "Chrono32";

#endif
SystemState stato;
// uint8_t db_array_value[4] = {0};
//  NexText Nset_temp         = NexText(0, 2, "Nset_temp");
//  NexText Ntcurr            = NexText(0, 3, "Ntcurr");
//  NexText Nout_temp         = NexText(0, 4, "Nout_temp");
//  NexCrop Nwater_on         = NexCrop(0, 5, "Nwater_on");
//  NexText Nout_hum          = NexText(0, 6, "Nout_hum");
//  NexText Nin_hum           = NexText(0, 7, "Nin_hum");
//  NexText Ncurr_hour        = NexText(0, 8, "Ncurr_hour");
//  NexText Nwater_temp  = NexText(0, 11, "Nwater_temp");
//  NexText Nday              = NexText(0, 12, "Nday");
//  NexButton Nb_up           = NexButton(0, 9, "Nb_up");
//  NexButton Nb_down         = NexButton(0, 10, "Nb_down");
//  NexCrop Nrisc_on          = NexCrop(0, 13, "Nrisc_on");
//  NexCrop Nalarm            = NexCrop(0, 14, "Nalarm");
//  NexTouch *nex_listen_list[] ={
//    &Nrisc_on,
//    &Nwater_on,
//    &Nalarm,
//    &Nb_up,
//    &Nb_down,
//    NULL
//  };
#ifdef DEBUG_UDP_LOG
WiFiUDP udpLog;

void udpLogBegin()
{
    udpLog.begin(UDP_LOG_PORT);
}

void udpLogSend(const char* msg)
{
    udpLog.beginPacket(UDP_LOG_IP, UDP_LOG_PORT);
    udpLog.write((uint8_t*)msg, strlen(msg));
    udpLog.endPacket();
}

void udpLogSend_f(const char* fmt, ...)
{
    if (m_wifi_status != CONN_OK) return;
    char buf[192];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    udpLogSend(buf);
}
#endif
