#include <Arduino.h>
#include "SPI.h"
#include <Ethernet.h>
#include <WebServer.h>
#include <PString.h>
#include <Time.h>

#include "wserver.h"

// This is our MAC address
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

/* This creates an instance of the webserver.  By specifying a prefix
 * of "/", all pages will be at the root of the server. */
#define PREFIX "/"
WebServer webserver(PREFIX, 80);

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
  webserver.addCommand("index.html", &helloCmd);

  /* start the webserver */
  webserver.begin();

}


void serviceServer() {

  char buff[64];
  int len = 64;
 
  /* process incoming connections one at a time forever */
  webserver.processConnection(buff, &len);

}

