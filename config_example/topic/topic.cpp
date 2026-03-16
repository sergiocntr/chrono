#include "topic.h"
//COMANDO DEVICE ESTERNI
const char* riscaldaTopic = "homie/esterno/caldaia/relay"; 
const char* acquaTopic ="homie/esterno/caldaia/acqua";  
const char* tendeTuyaCmd = "homie/tuya/tende/cmd";
//**********************************************************************************************************
//DATI IN INGRESSO
const char* updateTopic = "homie/update";
const char* logTopic ="homie/log"; 
const char* teleTopic = "homie/telegram";

const char* extSensTopic ="homie/esterno/sensori"; 
const char* powerTopic ="homie/esterno/caldaia/power"; 
const char* casaSensTopic = "homie/interno/sensori";

const char* systemTopic = "homie/general"; 
const char* eneValTopic ="homie/interno/mainP/value"; 

const char* tendeTuyaState = "homie/tuya/tende/state";