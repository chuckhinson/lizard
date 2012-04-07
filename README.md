Lizard is a system for monitoring the heating and air conditioning systems in my house.  

To build lizard, you will need the [Webduino] (https://github.com/sirleech/Webduino) library and the 
[Arduino Time Library](http://www.arduino.cc/playground/Code/Time).  My system currently runs on an Uno with 
a V5 Arduino Ethernet Shield.  It will also run on a Duemilanove with the original Arduino Ethernet Shield (my test system).

Lizard consist of the following components

 *    A network-attached arduino used to detect when any of the thermostats in my house are calling for heat or AC
 *    An application (ingester) to poll the arduino and store the data collected by the arduino in a database
 *    A web application for displaying and analyzing the data collected by the arduino
 *    A collection of relays in my basement that are hooked up to the thermostat inputs on my furnace and AC units.
      (I used this approach to protect my Arduino from the 24v AC power used by those systems)

There were two things that drove the conception of Lizard.  The first was my curiosity about how to determine 
what temperature to set my set-back thermostats to while no one is home.  The advantage of using a large set-back 
would be that the system would not run at all while the house is empty, but that might be offset by it having to 
run for a long time to get the house back to a comfortable temperature.  If I used a small set-back, the system 
would run more often while the house was empty, but it wouldnt take very long to return the house to a comfortable 
temperature.  I could not find any definitive advice, so trial and error seemed the only answer.

The second thing that motivated me to create this was that it would serve as a convenient playground for learning
some new programming languages and frameworks as well as learning a little about electronics.  

I am a programmer by trade and training.  A long time ago, I did some embedded systems work for the 
navigation and communications systems used in the president's helicopter.  I've not written any C code
since then, and since I never really learned C++, the arduino code may be a bit akward and ugly in places.

I've also had no training in electronics; everything I know about electronics came from the excellent (for me, at least)
Electronics Theory course on http://electonicstheory.com . I now know just enough to make a mess.

