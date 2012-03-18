
#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Time.h>

#include "ntptime.h"

void sendNtpPacket(IPAddress& address);
time_t parseNtpPacket();

time_t bootTime = 0;

/* Keeps track of our ntp processing state. Possible values are:
 * 0 = not initialized
 * 1 = initialized, waiting for sync interval
 * 2 = sync interval complete, waiting for time response
 */

static int ntpState = 0;
static time_t ntpLastSyncTime = 0;
static time_t syncInterval = 24 * 60 * 60;  // in seconds

static IPAddress timeServer(192, 43, 244, 18); // time.nist.gov NTP server

// Some of these IPs may change over time
static IPAddress ipaddresses[] = {
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

static int currentIP = 0;
static int maxRetries = 10;

static EthernetUDP Udp;
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

void ntptime_init() {

 
  int retryCount;
 
  Udp.begin(8888);

  ntpLastSyncTime = 0;
  setSyncProvider(0);  // We will take care of synching time ourselves,
                       // so tell time library there's no external time source
                       
  // Force an ntp request to be sent                       
  sendNtpPacket(ipaddresses[0]);                       
  ntpState = 2;
  
  // Wait until our request is completed
  // We'll retry until our watchdog resets
  retryCount = maxRetries;
  while (ntpState == 2) {
    if (retryCount-- <= 0) {
      currentIP = ++currentIP % (sizeof ipaddresses / sizeof (IPAddress));
      ipaddresses[0].printTo(Serial);
      sendNtpPacket(ipaddresses[currentIP]);                       
      Serial.print("Retrying ntp ");
      Serial.print(retryCount);
      retryCount = maxRetries;
    }  
    delay(500);
    serviceNtpTime();  
  }
  bootTime = now();

  ipaddresses[0].printTo(Serial);
  Serial.print("Time : ");
  Serial.println(bootTime);
  
}



/* This keeps our time up to date with our NTP time source */
void serviceNtpTime() {
  
  switch (ntpState) {
    case 1 :   
      if ((ntpLastSyncTime + syncInterval) < now()) {
        ntpState = 2;
        sendNtpPacket(timeServer);
        Serial.print("Sent time request");
      }
      break;
    case 2 :
      if ( Udp.parsePacket() ) {
        ntpLastSyncTime = parseNtpPacket();
        setTime(ntpLastSyncTime);
        ntpState = 1;        
        Serial.print("updated time");
      }
      break;
  }
  
}

time_t getBootTime() {
	return bootTime;
}

// send an NTP request to the time server at the given address 
void sendNtpPacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
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

time_t parseNtpPacket() {
  
  Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  

  unsigned long ntpTime = highWord << 16 | lowWord;  

  return (ntpTime - 2208988800UL);  // Unix time is seconds since Jan 1 1970
  
}


