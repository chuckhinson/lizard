#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Time.h>

#include "network.h"
#include "ntptime.h"
#include "wserver.h"

time_t lastNetworkRestartTime = 0;
int networkRestartCount = 0;

// This is our MAC address
//static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };


void startEthernet() {

  while (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP init failed");
	
	delay(5000);  //Sleep a bit and try again
  }

  Serial.print("IP addr: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();

}


void restartNetwork() {

	lastNetworkRestartTime = now();
	networkRestartCount++;

	startEthernet();
	
	initNtpTime();
	Serial.println("Ntp");

	initWebServer();
	Serial.println("Server");
	

}

void serviceNetwork() {

	// We've seen things freeze after a few days of being polled every minute
	// via http, so if we haven't seen a request for a while, we'll assume something's
	// up with the network and restart.  
	// We'd also like to refresh our DHCP lease every 24 hours.

	if ((now() - getLastRequestTime()) > (60*3)) {
		Serial.println("quiet");
		restartNetwork();
	} else if ((now() - lastNetworkRestartTime) > (24 * 60 * 60)) {
		Serial.println("24hr reset");
		restartNetwork();
	}
	
	serviceNtpTime();
	serviceWebServer();

}

void initNetwork() {

	startEthernet();

	initNtpTime();
	Serial.println("Ntp");
	lastNetworkRestartTime = now();

	initWebServer();
	Serial.println("Server");
  
}


