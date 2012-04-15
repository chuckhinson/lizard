
#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Time.h>

#include "ntptime.h"
#include "network.h"

void sendNtpPacket(IPAddress& address);
time_t parseNtpPacket();

static EthernetUDP Udp;
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 


/* Keeps track of our ntp processing state. Possible values are:
 * 0 = not initialized
 * 1 = initialized, waiting for sync interval
 * 2 = sync interval complete, waiting for time response
 */
static int ntpState = 0;
const int NTP_IN_SYNC = 1;
const int WAITING_FOR_RESPONSE = 2;

time_t lastNtpRequestTime  = 0;  // Time the last ntp request was made

const time_t MAX_WAIT_INTERVAL = 10L;	// Seconds to wait for an ntp request to 
										// complete before giving up

const time_t SYNC_INTERVAL = 6 * 60 * 60L;  // How long to wait (in seconds) before
											 // attempting to resync our time.  Testing
											 // has shown the arduino gaining a second
											 // every 24 hours, so we probably want to
											 // check more often than that (otherwise,
											 // when you do resync, time may go backwards,
											 // and that could cause issues with using the
											 // timestamps on relay events)
											 
static time_t ntpLastSyncTime = 0;	// time of our last successful ntp sync

static time_t bootTime = 0;	// Time we booted

// Some of these IPs may change over time
// TODO: Change this use use DNS names instead of hard-coded IP addresses.
static IPAddress servers[] = {
	IPAddress(192, 43, 244, 18),
	IPAddress(200,160,0,8),
	IPAddress(128,2,1,21),
	IPAddress(128,4,1,2),
	IPAddress(130,207,244,240),
	IPAddress(64,236,96,53),
	IPAddress(138,39,7,20),
	IPAddress(152,1,58,124),
	IPAddress(18,26,4,105),
	IPAddress(128,59,39,48)
};

static int currServerIndex = 0;

void sendTimeRequest() {

	lastNtpRequestTime = now();
	ntpState = WAITING_FOR_RESPONSE;
	Serial.print(currServerIndex);
	sendNtpPacket(servers[currServerIndex]);                       
	currServerIndex = ++currServerIndex % (sizeof servers / sizeof (IPAddress));
	Serial.print("Sent time request");

}

/*
 * Initialize our ntp client. 
 * 
 * We need ntp time in order to have a valid, up-to-date time
 * in order to create timestamps for the data we collect.  If we dont have
 * valid time, then there's no use returning since we cant collect data
 * without valid time. 
 *
 * Note, however, that now() will still keep track of time - it will just
 * be relative to when we booted.
 */
void initNtpTime() {

	Udp.begin(8888);

	setSyncProvider(0);  // We will take care of synching time ourselves,
					   // so tell time library there's no external time source

	sendTimeRequest();	// send an initial time request to kickstart things
	
	// Wait until our request is completed.  Note that this is initialization,
	// so if we dont have a good time value, we cant collect data because
	// we wont be able to create valid timestamps.
	int retryCount = 500;
	while (ntpState == WAITING_FOR_RESPONSE) {
		delay(500);
		serviceNtpTime();
		if (--retryCount <=0) {
			// We've been trying for a while to get the time.  Let's try
			// restarting in case the network didnt come up right last time.
			retryCount = 500;
			startEthernet();  // Let's try restarting the ethernet
			restartNtpTime();
		}
	}

	if (bootTime == 0) {
	bootTime = now();
	}

  
}

/*
 * Restart our ntp handler.  This is only done as part of restarting the
 * network.  As such, all we want to do is make sure udp is still working.
 * We dont need to issue a time request unless one was already outstanding
 * (not likely)
 */
void restartNtpTime() {

	Udp.stop();

	Udp.begin(8888);

	// If we were already waiting for a response, let's re-issue the request
	if (ntpState == WAITING_FOR_RESPONSE) {
		sendTimeRequest();
	}

}

/*
 * Perform housekeeping needed to keep ntp time up to date.
 *
 * This essentially involves occasionally re-syncing our
 * time with a time server so our internal clock doesnt
 * drift too far.
 *
 */
void serviceNtpTime() {
  
	switch (ntpState) {
		case NTP_IN_SYNC :   
			// Check to see if we need to re-sync
			if ((ntpLastSyncTime + SYNC_INTERVAL) < now()) {
				sendTimeRequest();
			}
			break;
		case WAITING_FOR_RESPONSE :
			// See if we've received a response to a sync request yet
			if ( Udp.parsePacket() ) {
				ntpLastSyncTime = parseNtpPacket();
				setTime(ntpLastSyncTime);
				ntpState = NTP_IN_SYNC;
				Serial.print("Updated time: ");
				Serial.println(now());
			} else if ((lastNtpRequestTime + MAX_WAIT_INTERVAL) < now()) {
				// We've been waiting a while now and we're probably not going
				// to get a response, so let's try the next server in the list
				Serial.println("No ntp response");
				sendTimeRequest();
			}
			break;
	}
  
}

time_t getBootTime() {
	return bootTime;
}

/*
 * Send an NTP request to the time server at the given address 
 *
 * This code was lifted from sample code that was part of an
 * arduino ntp libary.
 */
// 
void sendNtpPacket(IPAddress& address)
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE); 

	// Initialize values needed to form NTP request
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision

	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49; 
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:         
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer,NTP_PACKET_SIZE);
	Udp.endPacket(); 
  
}

/*
 * Send an NTP request to get the current time.
 *
 * This code was lifted from sample code that was part of an
 * arduino ntp libary.
 */
time_t parseNtpPacket() {
  
  Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  

  unsigned long ntpTime = highWord << 16 | lowWord;  

  return (ntpTime - 2208988800UL);  // NTP time is based on 01-Jan-1900, but
									// we want Unix time which is seconds 
									// since 01-Jan-1970
  
}


