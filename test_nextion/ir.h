#ifndef irr_h
#define irr_h
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
const uint16_t kRecvPin = 14;
const uint16_t kCaptureBufferSize = 256;
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results;
void startIr(){
  irrecv.enableIRIn();  // Start the receiver
}
#endif
void irRoutine()
{
  if (irrecv.decode(&results))
  {
    uint64_t infraredNewValue = results.value;
    switch (infraredNewValue)
    {
    case monnezza:
      mqttWifi::publish(teleTopic, "monnezza");
      break;
    case spegni:
      mqttWifi::publish(teleTopic, "spegni");
      delay(10);
      LOG_VERBOSE("[irRoutine] SHUTDOWN_FROM_MQTT!");
      mqttWifi::adessoDormo(8, MotivoSpegnimento::SHUTDOWN_FROM_MQTT);
      break;
    case acquaON:
      mqttWifi::publish(acquaTopic, "1");
      break;
    case eneOff:
      mqttWifi::publish(eneTopic, "0");
      break;
    default:
      String s = int64String(infraredNewValue, HEX, false);
      mqttWifi::publish(iRTopic, s.c_str());
      break;
    }
    yield();
    irrecv.resume(); // Receive the next value
  }
}
