#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Time.h>

#include <WebServer.h>
#include <PString.h>

#include "ntptime.h"
#include "wserver.h"
#include "relay.h"

static int counter = 0;
static int relayState = 0;

void setup()
{
  
  Serial.begin(9600);

  Serial.println("Startup");
  
  server_init();
  Serial.println("Server initialized");
  
  ntptime_init();
  Serial.println("Ntp initialized");

  relay_init();
  Serial.println("Relays initialized");
  
}


void loop() 
{
  unsigned long delayTime;
  int state;
  
 
  /* Keep time up to date */
  serviceNtpTime();

  time_t t = now();

  serviceRelay();
  state = getRelayState();
  if (state != relayState) {
    Serial.print(hour(t));  
    Serial.print(":");
    Serial.print(minute(t));
    Serial.print(":");
    Serial.println(second(t));
    Serial.print("Relay state: ");
    Serial.println(state);
    relayState = state;
  }
  
  /* process incoming connections one at a time forever */
  /* This should be done last so as to have latest info gathered on this iteration */
  serviceServer();

  delayTime = 1000 - (millis() % 1000);
  delay(delayTime);
  
   
//  Serial.print("millis: ");
//  Serial.println(millis());

}

