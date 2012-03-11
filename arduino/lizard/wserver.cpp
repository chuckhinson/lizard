#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <WebServer.h>
#include <PString.h>
#include <Time.h>

#include "wserver.h"
#include "events.h"

void printJsonForEvent(char *, int, event_t *);

// This is our MAC address
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

/* This creates an instance of the webserver.  
 * PREFIX must not end with a '/'.  If you want everything at the
 * root, use an empty prefix ("")
 */
#define PREFIX ""
WebServer webserver(PREFIX, 80);

static char jsonEventFormat[] = "{\"seconds\":%ld,\"millis\":%d,\"deviceId\":%d,\"value\":%d}";

/* commands are functions that get called by the webserver framework
 * they can read any posted data from client, and they output to the
 * server to send data back to the web browser. */
void helloCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  char buffer[64];
  PString mystring(buffer, sizeof(buffer));
  
  /* this line sends the standard "we're all OK" headers back to the
     browser */
  server.httpSuccess();

  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type != WebServer::HEAD)
  {
    mystring = "Current Time: ";
    mystring.print(hour(now()));
    mystring += ":";
    mystring.print(minute(now()));
    mystring += ":";
    mystring.println(second(now()));
    
    server.print(mystring);

  }

}

void eventsCmd(WebServer &server, WebServer::ConnectionType type, char*, bool)
{

	char buffer[sizeof(event_t) * MAX_EVENTS];
	event_t *event;
	char comma = ' ';
	
	server.httpSuccess("application/json");
	time_t currSeconds = 0;
	uint16_t currMillis = 0;
	
	server.print("{\"events\":[");
	while ((event=getNextEvent(currSeconds,currMillis)) != 0) {
	    if (comma != ' ') {
			server.print(comma);
		}
		currSeconds = event->seconds;
		currMillis = event->millis;
		printJsonForEvent(buffer, sizeof(buffer), event);
		server.print(buffer);
		comma = ',';
	}
	server.print("]}");

}

void testCmd(WebServer &server, WebServer::ConnectionType type, char*, bool)
{

	
	server.httpSuccess("application/json");

	server.print("{\"dateTimeForma\":\"mmm dd yyy HH:MM:ss GMT\",\"events\":[]}");
}


void printJsonForEvent(char* buf, int bufSize, event_t *event) {

	PString mystring(buf, bufSize);
	mystring.format(jsonEventFormat,
					event->seconds,
					event->millis,
					event->inputId,
					event->value);
	return;


}

void server_init() {
  
 
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }

  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  
    /* setup our default command that will be run when the user accesses
   * the root page on the server */
  webserver.setDefaultCommand(&helloCmd);

  /* run the same command if you try to load /index.html, a common
   * default page name */
  webserver.addCommand("index", &helloCmd);
  webserver.addCommand("events", &eventsCmd);
  webserver.addCommand("test", &testCmd);

  /* start the webserver */
  webserver.begin();

}


void serviceServer() {

  char buff[64];
  int len = 64;
 
  /* process incoming connections one at a time forever */
  webserver.processConnection(buff, &len);

}

