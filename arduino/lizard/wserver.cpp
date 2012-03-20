#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <WebServer.h>
#include <PString.h>
#include <Time.h>
#include <avr/pgmspace.h>

#include "wserver.h"
#include "ntptime.h"
#include "relay.h"
#include "events.h"
#include "status_css.h"

void printJsonForEvent(char *, int, event_t *);
void printInfoColumn(WebServer &server);
void printIndicatorColumn(WebServer &server);
void printIndicator(WebServer &server, int num);

// This is our MAC address
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

/* This creates an instance of the webserver.  
 * PREFIX must not end with a '/'.  If you want everything at the
 * root, use an empty prefix ("")
 */
#define PREFIX ""
WebServer webserver(PREFIX, 80);

static char jsonEventFormat[] = "{\"seconds\":%ld,\"millis\":%d,\"deviceId\":%d,\"value\":%d}";


const prog_uchar status_doc_part1[] PROGMEM = 
"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
                 "\"http://www.w3.org/TR/html4/loose.dtd\">"
                 "<html>"
                 "<head>"
				 "<meta http-equiv=\"refresh\" content=\"5\">";
				 
const prog_uchar status_doc_part2[] PROGMEM = 
"<title>Lizard System Status</title>"
                   "</head>"
                   "<body>"
                   "<div class=\"header\">"
                   "<h1>Lizard System Status</h1>"
                   "</div>"
                   "<div class=\"columns\">";				 
				 
/* commands are functions that get called by the webserver framework
 * they can read any posted data from client, and they output to the
 * server to send data back to the web browser. */
void helloCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  
  /* this line sends the standard "we're all OK" headers back to the
     browser */
  server.httpSuccess();
  
  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type != WebServer::HEAD)
  {
	server.printP(status_doc_part1);
				 
	server.printP(status_css);
	
	server.printP(status_doc_part2);
	
	printInfoColumn(server);
	
	printIndicatorColumn(server);
	
    server.println("</div><body></html>");

  }

}

void printInfoColumn(WebServer &server) {

    char buffer[64];
    PString mystring(buffer, sizeof(buffer));

	server.println("<div class=\"info-column\">");
	server.print("<p>Last boot: ");

	time_t t = getBootTime();
    mystring.print(hour(t));
    mystring += ":";
    mystring.print(minute(t));
    mystring += ":";
    mystring.print(second(t));
	mystring += "GMT ";
	mystring.print(day(t));
	mystring +="-";
	mystring.print(monthShortStr(month(t)));
	mystring +="-";
	mystring.print(year(t));
    server.print(mystring);
	
	server.print("</p><p>IP address: ");
	mystring.begin();
	for (byte thisByte = 0; thisByte < 4; thisByte++) {
		// print the value of each byte of the IP address:
		mystring.print(Ethernet.localIP()[thisByte], DEC);
		mystring +="."; 
	}
	server.println(mystring);
	
	server.println("</p></div>");

}

void printIndicatorColumn(WebServer &server) {
	int i;
	
	server.println("<div class=\"indicator-column\">");
	server.println("<ul class=indicator-list>");

	for (i=0; i<MAX_LINES; i++) {
		printIndicator(server, i);
	}
	
	server.println("</ul>\n</div>");

}


void printIndicator(WebServer &server, int num) {

    char buffer[16];
    PString mystring(buffer, sizeof(buffer));
	
	time_t t = getLastEventTime(num);
	int state = getLastEventState(num);
	
    server.println("<li>");
    server.println("<div class=\"indicator\">");
    server.print  ("<span class=\"off-indicator ");
	if (state == 0) server.print("off-active");
	server.print("\">OFF</span><span class=\"on-indicator ");
	if (state != 0) server.print("on-active");
    server.print("\">ON</span><span class=\"event-time\">");

    mystring.print(hour(t));
    mystring += ":";
    mystring.print(minute(t));
    mystring += ":";
    mystring.print(second(t));
	mystring += " GMT";
    server.print(mystring);
	
    server.println("</span></div>");
	server.print  ("<span class=\"indicator-name\">Line ");
	server.println(num+1);
	server.println("</span>\n</li>");

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

/*
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
*/

/*
void testCmd(WebServer &server, WebServer::ConnectionType type, char*, bool)
{
	server.httpSuccess("application/json");

	server.print("{\"dateTimeFormat\":\"mmm dd yyy HH:MM:ss GMT\",\"events\":[]}");
}
*/

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
//  webserver.addCommand("events", &eventsCmd);
//  webserver.addCommand("test", &testCmd);

  /* start the webserver */
  webserver.begin();

}


void serviceServer() {

  char buff[64];
  int len = 64;
 
  /* process incoming connections one at a time forever */
  webserver.processConnection(buff, &len);

}

