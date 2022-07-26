#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
namespace tempDHT{
#define DHTPIN D4     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
uint32_t delayMS;
DHT_Unified dht(DHTPIN, DHTTYPE);
void setupTemp(){
    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    delayMS = sensor.min_delay / 1000;
}
void getLocalTemp(){
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  // sensors_event_t event;
  // dht.temperature().getEvent(&event);
  // if (isnan(event.temperature)) {
  //   Serial.println(F("Error reading temperature!"));
  // }
  // else {
  //   Serial.print(F("Temperature: "));
  //   Serial.print(event.temperature);
  //   Serial.println(F("°C"));
  // }
  // // Get humidity event and print its value.
  // dht.humidity().getEvent(&event);
  // if (isnan(event.relative_humidity)) {
  //   Serial.println(F("Error reading humidity!"));
  // }
  // else {
  //   Serial.print(F("Humidity: "));
  //   Serial.print(event.relative_humidity);
  //   Serial.println(F("%"));
  // }
  static uint8_t volteTemp = 0;
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    //Serial.println(F("Error reading temperature!"));
    String err = "Error reading temperature!";
    mqttWifi::publish(logTopic,err.c_str());
    Ntcurr.setText("TFAIL");
    return;
  }
  float temp = (float)event.temperature;
  myTemp.t += temp;
  volteTemp++;
  delay(10);
  
  dht.humidity().getEvent(&event);
  float hum = (float)event.relative_humidity;
  myTemp.h += hum;
  delay(10);
  if(volteTemp > 3 ){
    volteTemp = 0;
    mqttWifi::sendData();

    if(!mqttWifi::mqttOK)
    {
      mqttWifi::adessoDormo();
    }
    
  }
}  
}