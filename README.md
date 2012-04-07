Lizard is a system for monitoring the heating and air conditioning systems in my house.  

Lizard consist of the following components

 *    A network-attached arduino used to detect when any of the thermostats in my house are calling for heat or AC
 *    An application (ingester) to poll the arduino and store the data collected by the arduino in a database
 *    A web application for displaying and analyzing the data collected by the arduino
 *    A collection of relays in my basement that are hooked up to the thermostat inputs on my furnace and AC units.
      (I used this approach to protect my Arduino from the 24v AC power used by those systems)

Currently, only the arduino and ingester portions of the system exist.  

##Arduino##
The arduino collects data in the form of events.  An event is recorded any time the state of one of the thermostat 
relays changes.  An event records the time of the change, the id of the relay that changed and the new state of the relay.

The arduino provides access to the event data that it collects via an http server.  There are three endpoints defined in
the http server:

 *    An html status page which provides a graphical display of the current state oethe relays along with some other
      status information.  This endpoint exists primarily to allow humans to see that current state of the relays.
      As lizard also contains a web-based application for viewing and analying the collected data, it is expected
      that this status page would only be used when troubleshooting the system.
 *    A JSON-formatted version of the status page. This endpoint exists because the arduino does not have enough 
      memory to host the sort of browser-based application that I'd like to have for displaying the current status
      of the relays/thermostats.  Having this endpoint will allow me to host the web page elsewhere, but still
      get the status data directly from the arduino in real-time.
 *    An events page, formatted as JSON, that contains a list of the last n events that were collected.  This is 
      the endpoint that the ingester will use to retrieve the event data to be stored in the database.

In addition to network and power connections, the arduino can be connected to up to seven relays via an RJ-45 
connector.  Pins 0 - 7 of the RJ-45 jack are connected to digital I/O pins on the arduino, and pin 8 of the jack
is connected to the arduino's ground.  The arduino digital I/O pins connected to the RJ-45 jack have their pull-up
resistors enabled so that the pins are are pulled high while the relays are open and then go to ground when relays close. 
The relays are driven by the 24 VAC signal coming from the thermostats in the house. (There are three heating signals and
two cooling signals.  I also intend to have the aquastat in the water heater drive another relay.)

To build the arduino portion of lizard, you will need the [Webduino] (https://github.com/sirleech/Webduino) library and the 
[Arduino Time Library](http://www.arduino.cc/playground/Code/Time).  My system currently runs on an Uno with 
a V5 Arduino Ethernet Shield.  It will also run on a Duemilanove with the original Arduino Ethernet Shield (my test system).


##Ingester##
The ingester is http client software that polls the arduino for event data and stores the data in a database
for further processing.  It is intended to run continuously as a background process on a computer that is 
turned on all the time.

An initial version of the ingester has been written in Groovy and stores the data in an HSQL database.  Because 
the arduino has a limited amount of RAM, the ingester needs to poll the arduino often enough so that old event 
data does not get pushed out of the arduino's buffer before it can be collected.  

The ingester currently polls the arduino for data every minute.  While one would not expect thermostats to cycle on and 
off very rapidly, the iniial data I've collected shows my thermostats cylcing on and off every three to five minutes 
in cold weather (we have gas hot-water heat). At that rate, a sample period of more than ten minutes would probably be
too long for the worst-case scenario as the arduino currently only has room to store the last 16 events (each thermostat 
cylcing on and off every three minutes could give up to 18 events in 15 minutes).


##Background##
There were two things that drove the conception of Lizard.  The first was my curiosity about how to determine 
what temperature to set my set-back thermostats to while no one is home.  The advantage of using a large set-back 
would be that the system would not run at all while the house is empty, but that might be offset by it having to 
run for a long time to get the house back to a comfortable temperature.  If I used a small set-back, the system 
would run more often while the house was empty, but it wouldnt take very long to return the house to a comfortable 
temperature.  I could not find any definitive advice, so trial and error seemed the only answer.

The second thing that motivated me to create this was that it would serve as a convenient playground for learning
some new programming languages and frameworks as well as learning a little about electronics.  

I am a programmer by trade and training.  A long time ago, I wrote debuggers, device drivers and other OS-level code.
I even did some embedded systems work on the navigation and communications systems used in the president's helicopter.
I've not written any C code since then, and since I never really learned C++, the arduino code may be a bit akward 
and ugly in places.  Java, Groovy, and Javascript (and soon Scala) are the languages I currently use.

I've also had no training in electronics; everything I know about electronics came from the excellent (for me, at least)
Electronics Theory course on http://electonicstheory.com . I now know just enough to make a mess.

