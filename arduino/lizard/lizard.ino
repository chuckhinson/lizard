#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Time.h>

#include <WebServer.h>
#include <PString.h>

#include "network.h"
#include "relay.h"
#include "events.h"


void setup()
{
	  
	Serial.begin(9600);

	Serial.println("Startup");
  
	initNetwork(); // want to do this first so we have correct time
	
	initRelays();
	Serial.println("Relays");
  
	initEvents();
	Serial.println("Events");
	
}


void loop()
{
	serviceRelays();

	processRelayEvents(getLineState());

	serviceNetwork();

	// Duty cycle of 10 Hz just so the web server will be responsive
	// to incoming requests - otherwise we could go slower since we
	// dont expect lots of activity on the relay lines.
	delay(100 - (millis() % 100));

}

