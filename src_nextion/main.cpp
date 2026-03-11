#include "main.h"
void setup(){
    delay(1000);
    NexManager::begin(38400);
    
}
void loop(){
        // 2. Poll Nextion - ora ritorna l'evento
    NexManager::TouchEvent evt = NexManager::poll();
    if (evt.isValid)
    {
      switch (evt.page)
      {
      case 0:
        handleHomePage(evt);
        break;
      case 1:
        handleTendePage(evt);
        break;
      default:
        break;
      }
    }
    delay(10);
}