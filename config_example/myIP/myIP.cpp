#include <myIP.h>
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(192, 168, 1, 1);
IPAddress dns2(8, 8, 8, 8);
///////////////////////////////////////////////////////////
IPAddress ipMqtt_server(192, 168, 1, 100);
IPAddress ipChrono(192, 168, 1, 107);
//////////////////////////////////////////////////////////
const char *chronoId = "Chrono";
const uint16_t mqtt_port = 1883;