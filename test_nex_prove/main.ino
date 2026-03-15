#include <ESP8266WiFi.h>
#include "SPI.h"               //package builtin configuration file
#include "SD.h"               //package builtin configuration file
#include "Nextion.h"
NexText Nday                = NexText(0, 7, "NtxtDay");
NexText NtxtStanza          = NexText(0, 5, "NtxtStanza");
NexText NtxtAcq             = NexText(0, 6, "NtxtAcq");
NexText NtxtOra             = NexText(0, 4, "NtxtOra");
NexCrop NcrAcq              = NexCrop(0, 2, "NcrAcq");
NexCrop NcrRis              = NexCrop(0, 1, "NcrRis");
NexCrop NcrAlert            = NexCrop(0, 3, "NcrAlert");
NexTouch *nex_listen_list[] ={
  &NcrRis,
  &NcrAcq,
  &NcrAlert,
  NULL
};
boolean AlarmOn = false; // ALLARME ACQUA IN SPEGNIMENTO

void setup()
{
 nex_routines();
}
void loop(){
sendCommand("picq 163,24,205,168,1");
delay(250);
sendCommand("picq 163,24,205,168,0");
delay(250);
sendCommand("picq 163,24,205,168,1");
delay(250);
sendCommand("picq 163,24,205,168,0");
delay(250);
sendCommand("picq 163,24,205,168,1");
delay(250);
sendCommand("picq 163,24,205,168,0");
delay(250);
sendCommand("picq 163,24,205,168,0");
delay(250);
sendCommand("ref 5");
delay(10);
sendCommand("ref 6");
delay(3000);
}

void nex_routines(){
  nexInit();
  delay(10);
  sendCommand("dim=20");
  //Nset_temp.setText("");
  NtxtOra.setText("");
  // Nout_temp.setText("");
  // Nout_hum.setText("");
  // Nin_hum.setText("");
  // Ncurr_hour.setText("");
  NtxtAcq.setText("");
  Nday.setText("");
  //NcrRis.attachPush(Nrisc_onPushCallback);
  //NcrAcq.attachPush(Nwater_onPushCallback);

  //update_buttons();
  sendCommand("sleep=0");
}