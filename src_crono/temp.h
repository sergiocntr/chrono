#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
namespace tempDHT
{
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 22 (AM2302)
  uint32_t delayMS;
  DHT_Unified dht(DHTPIN, DHTTYPE);
  static uint8_t volteTemp = 0;
  void setupTemp()
  {
    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    delayMS = sensor.min_delay / 1000;
  }
  void getLocalTemp() {
    delay(delayMS);  // rispetta intervallo minimo
    delay(250);
    sensors_event_t eventT, eventH;
    dht.temperature().getEvent(&eventT);
    dht.humidity().getEvent(&eventH);
  
    if (isnan(eventT.temperature) || isnan(eventH.relative_humidity)) {
      mqttWifi::publish(logTopic, "chrono2: Error reading temp/hum");
      Ntcurr.setText("TFAIL");
      return;
    }
  
    myTemp.t += eventT.temperature;
    myTemp.h += eventH.relative_humidity;
    volteTemp++;
  
    if (volteTemp >= 4) {
      volteTemp = 0;
      mqttWifi::sendData();  // presumibilmente media e invio
    }
  }
}