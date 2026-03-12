#include "main.h"
void setup(){
    delay(4000);
    NexManager::begin(38400);
    delay(200);
    NexManager::sendCommand("Nwater_on.picc=1");
    delay(2000);
    NexManager::sendCommand("Nwater_on.picc=0");
    
}
void loop(){
        // 2. Poll Nextion - ora ritorna l'evento
    NexManager::TouchEvent evt = NexManager::poll();
    // if (evt.isValid)
    // {
    //   switch (evt.page)
    //   {
    //   case 0:
    //     handleHomePage(evt);
    //     break;
    //   case 1:
    //     handleTendePage(evt);
    //     break;
    //   default:
    //     break;
    //   }
    // }
    delay(10);
}