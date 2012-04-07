
#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Time.h>

#include "ntptime.h"

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

const int MAX_RETRIES = 10;
const time_t SYNC_INTERVAL = 24 * 60 * 60;  // in seconds
static time_t ntpLastSyncTime = 0;

static time_t bootTime = 0;

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


void initNtpTime() {

  int retryCount;

  Udp.stop();   // in case we're restarting
  
  Udp.begin(8888);

  ntpLastSyncTime = 0;
  
  setSyncProvider(0);  // We will take care of synching time ourselves,
                       // so tell time library there's no external time source

                       
  sendNtpPacket(servers[0]);
  ntpState = WAITING_FOR_RESPONSE;

  // Wait until our request is completed
  // We'll retry until our watchdog resets
  retryCount = MAX_RETRIES;
  while (ntpState == WAITING_FOR_RESPONSE) {
    if (retryCount-- <= 0) {
      currServerIndex = ++currServerIndex % (sizeof servers / sizeof (IPAddress));
      servers[currServerIndex].printTo(Serial);
      sendNtpPacket(servers[currServerIndex]);                       
      Serial.print("Retrying ntp ");
      Serial.print(retryCount);
      retryCount = MAX_RETRIES;
    }  
    delay(500);
    serviceNtpTime();  
  }
  
  if (bootTime == 0) {
	bootTime = now();
  }

  Serial.print("Time : ");
  Serial.println(now());
  
}

/*
 * Perform housekeeping needed to keep ntp time up to date.
 *
 * This essentially involves occasionally re-syncing our
 * time with a time server so our internal clock doesnt
 * drift too far.
 *
 * Note that due to the way we're handling network resets,
 * this function is not likely to ever actaully issue
 * an ntp request - that will almost always happen from
 * the init routine
 */
void serviceNtpTime() {
  
  switch (ntpState) {
    case NTP_IN_SYNC :   
	  // Check to see if we need to re-sync
      if ((ntpLastSyncTime + SYNC_INTERVAL) < now()) {
        ntpState = WAITING_FOR_RESPONSE;
		currServerIndex = ++currServerIndex % (sizeof servers / sizeof (IPAddress));
		sendNtpPacket(servers[currServerIndex]);                       
        Serial.print("Sent time request");
      }
      break;
    case WAITING_FOR_RESPONSE :
	  // See if we've received a response to a sync request yet
      if ( Udp.parsePacket() ) {
        ntpLastSyncTime = parseNtpPacket();
        setTime(ntpLastSyncTime);
        ntpState = NTP_IN_SYNC;
        Serial.println("updated time");
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


