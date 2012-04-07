#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <WebServer.h>
#include <PString.h>
#include <Time.h>
#include <avr/pgmspace.h>

#include "network.h"
#include "wserver.h"
#include "ntptime.h"
#include "relay.h"
#include "events.h"
#include "status_html.h"


/* This creates an instance of the webserver.  
 * PREFIX must not end with a '/'.  If you want everything at the
 * root, use an empty prefix ("")
 */
#define PREFIX ""
WebServer webserver(PREFIX, 80);


static char jsonEventFormat[] = "{\"seconds\":%ld,\"millis\":%d,\"deviceId\":%d,\"value\":%d}";
static char jsonStatusFormat[] = "\"bootTime\":%ld,\"lastNetRestartTime\":%ld,\"netRestartCount\":%d";
static char jsonLineStatusFormat[] = "{\"deviceId\":%d,\"seconds\":%ld,\"value\":%d}";

static char timeFormat[] = "%02d:%02d:%02d";

// Used to keep track of how many http requests we satisfy
static int requestCounter = 0;
static time_t lastRequestTime = 0;


time_t getLastRequestTime() {
	return lastRequestTime;
}

int getRequestCount() {
	return requestCounter;
}

void formatDateTime(PString &mystring, time_t t)
{
	mystring.format(timeFormat, hour(t), minute(t), second(t));
	mystring += "GMT ";
	mystring.print(day(t));
	mystring +="-";
	mystring.print(monthShortStr(month(t)));
	mystring +="-";
	mystring.print(year(t));
}

void printInfoColumn(WebServer &server) {

    char buffer[32];  // Be sure to adjust if you change what gets printed with pstring
    PString mystring(buffer, sizeof(buffer));

	server.printP(status_col_line1);
	formatDateTime(mystring, getBootTime());
    server.print(mystring);
	
	server.printP(status_col_line2);
	mystring.begin();
	formatDateTime(mystring, lastNetworkRestartTime);
	server.println(mystring);
	
	server.printP(status_col_line3);
	mystring.begin();
	mystring.print(networkRestartCount, DEC);
	server.println(mystring);

	server.println("</p></div>");

}

void printIndicator(WebServer &server, int num) {

    char buffer[16];
    PString mystring(buffer, sizeof(buffer));
	
	time_t t = getLastEventTime(num);
	int state = getLastEventState(num);
	
    server.printP(indicator_line_start);
	if (state == 0) server.print("off-active");
	server.printP(indicator_line_off);
	if (state != 0) server.print("on-active");
    server.printP(indicator_line_on);

	mystring.format(timeFormat, hour(t), minute(t), second(t));
	mystring += " GMT";
    server.print(mystring);
	
    server.printP(indicator_line_name);
	server.println(num+1);
	server.printP(indicator_line_end);

}

void printIndicatorColumn(WebServer &server) {
	int i;
	
	server.printP(indicator_col_start);

	for (i=0; i<MAX_LINES; i++) {
		printIndicator(server, i);
	}
	
	server.printP(indicator_col_end);

}


void indexCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  requestCounter++;
  lastRequestTime = now();
  
  /* this line sends the standard "we're all OK" headers back to the
     browser */
  server.httpSuccess();
  
  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type != WebServer::HEAD)
  {
	server.printP(status_doc_prolog);
				 
	printInfoColumn(server);
	
	printIndicatorColumn(server);
	
    server.printP(status_doc_epilog);

  }

}


void eventsCmd(WebServer &server, WebServer::ConnectionType type, char*, bool)
{
    requestCounter++;
    lastRequestTime = now();

	char buffer[64];  // large enought to hold one json event
    PString mystring(buffer, sizeof(buffer));
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
		mystring.format(jsonEventFormat,
						event->seconds,
						event->millis,
						event->inputId,
						event->value);
		server.print(mystring);
		comma = ',';
	}
	server.print("]}");

}


void statusCmd(WebServer &server, WebServer::ConnectionType type, char*, bool)
{

    requestCounter++;
    lastRequestTime = now();

	char buffer[80];
	PString mystring(buffer, sizeof(buffer));
	char comma = ' ';
	int i;
	
	server.httpSuccess("application/json");

	server.print("{");
	
	mystring.begin();
	mystring.format(jsonStatusFormat,getBootTime(), lastNetworkRestartTime, networkRestartCount);
	server.print(mystring);

	mystring.print(networkRestartCount, DEC);
	
	server.print(",\"lines\":[");
	for (i=0;i<MAX_LINES;i++) {
	    if (comma != ' ') {
			server.print(comma);
		}
		mystring.begin();
		mystring.format(jsonLineStatusFormat, i, getLastEventTime(i), getLastEventState(i));
		server.print(mystring);
		comma = ',';
	}
	server.print("]}");


}
/*
void testCmd(WebServer &server, WebServer::ConnectionType type, char*, bool)
{
    requestCounter++;
    lastRequestTime = now();
	
	server.httpSuccess("application/json");

	server.print("{\"dateTimeFormat\":\"mmm dd yyy HH:MM:ss GMT\",\"events\":[]}");
}
*/

void initWebServer() {
  
 	webserver.setDefaultCommand(&indexCmd);
	webserver.addCommand("index", &indexCmd);
	webserver.addCommand("events", &eventsCmd);
	webserver.addCommand("status", &statusCmd);
	//  webserver.addCommand("test", &testCmd);

	/* start the webserver */
	webserver.begin();

    lastRequestTime = now();

}

void serviceWebServer() {

  char buff[64];
  int len = sizeof(buff);
 
  /* process incoming connections one at a time forever */
  webserver.processConnection(buff, &len);

}

