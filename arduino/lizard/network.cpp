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

/*
 * Start up the ethernet stack.
 *
 * This is primarily to get the Wiznet chip initialized and
 * then to get ourselves an IP address via DHCP.
 */
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

/*
 * Restart the networking stack.
 *
 * In testing, we've observed that things lock up after a day or
 * two of continous polling.  Restarting the ethernet stack occasionally
 * seems to take care of that problem.
 *
 * Also, DHCP leases typically expire after 24 hours, so we need to
 * update our lease to make sure our IP address is given away to some
 * other device on the network. Note that we take a lazy/brute-force 
 * approach and restart the entire networking stack rather than just 
 * updating the DHCP lease.
 *
 */
void restartNetwork() {

	lastNetworkRestartTime = now();
	networkRestartCount++;

	startEthernet();
	
	// Since we've restarted the ethernet stack, we'll need to restart
	// any network services to ensure they're not using old sockets
	
	restartNtpTime();
	Serial.println("Ntp");

	initWebServer();
	Serial.println("Server");
	
}

/*
 * Take care of any processing related to networking.
 * 
 * This includes keeping our time up to date (ntp) and
 * servicing any incoming http requests.  Note that we
 * also check to see if we need to restart the network
 * in case of errors or to renew our DHCP lease.
 * 
 */
void serviceNetwork() {


	if ((now() - getLastRequestTime()) > (10*60L)) {
		// It's been a while since we've seen a network request.  Since
		// we're expecting to be polled continuously, we'll assume there's
		// something wrong with the network stack and restart it.
		Serial.println("quiet");
		restartNetwork();
	} else if ((now() - lastNetworkRestartTime) > (24 * 60 * 60L)) {
		// It's been 24 hourse since the last network reset.  Let's restart
		// the network in order to update our DHCP lease.
		Serial.print("24hr reset - ");
		Serial.println(now());
		restartNetwork();
	}
	
	serviceNtpTime();
	
	serviceWebServer();

}

/*
 * Initialize the network stack.
 *
 * This includes setting getting an IP address via DHCP, getting
 * the current time fron an NTP server and initializing our 
 * web server.
 */
void initNetwork() {

	startEthernet();

	initNtpTime();
	Serial.println("Ntp");
	lastNetworkRestartTime = now();

	initWebServer();
	Serial.println("Server");
  
}


