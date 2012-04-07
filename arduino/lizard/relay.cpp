#include <Arduino.h>
#include <Time.h>

#include "relay.h"

const int PIN_COUNT  = 16;  // The Arduino only has 13 pins, but we'll leave room for 16
							// just because it's a nice round binary number

/*
 * The ethernet shield uses the SPI bus to communicate - this uses digital io pins 11,12, 
 * and 13.  It also uses pin 10 to select the ethernet chip and pin 4 to select the SD 
 * card (since we dont use the SD card I'd think we should be able to use pin 4 for input).  
 * Pins 0 and 1 are the TX and RX pins for the board's serial port, so we cant expect to 
 * use those if we still want to do serial debugging.  Which leaves us able to use pins 2 - 9
 *
 * Value is a bitmask, 1 bits indicate pin is active/usable.  
 */
 //                                        1 1 1 1  1 1 
 //                           pin number:  5 4 3 2  1 0 9 8  7 6 5 4  3 2 1 0
 //                                        -----------------------------------
static uint16_t activePins = 0x03FC;    // 0 0 0 0  0 0 1 1  1 1 1 1  1 1 0 0


// There are 8 lines coming into the ethernet jack that we have connected to the relays.
// Since we want the rest of the system to think in terms of lines rather than I/O pins,
// we need to map the pins to their corresponding lines.  Note that line 8 is actually
// tied to ground, so we really only have 7 available lines coming in.
//                                                              1   1   1   1   1   1 
//                       pin   0   1  2  3  4  5  6  7  8   9   0   1   2   3   4   5
static int pinToLineMap[] = { -1, -1, 0, 1, 2, 3, 4, 5, 6, -1, -1, -1, -1, -1, -1, -1};
 
// debounce sample ring buffer
const int RING_SIZE = 3;
static uint16_t stateRing[ RING_SIZE ];
static uint16_t ringIndex = 0;

// Current IO pin (relay) state as of latest read
static int state = 0;


/*
 * Init our active pins for input
 */
void initRelays() {
  int i;
  
  for (i=0; i<PIN_COUNT; i++) {
	uint16_t mask = (1 << i);
	if (mask & activePins) {
	  pinMode(i, INPUT);
      digitalWrite(i,HIGH);
	}
  }
  
}


/*
 * Read (and debounce) the current state of the attached relays.
 * Note that when a relay is closed, we treat that as true or 1,
 * and when it's open, we treat that as false or 0.
 */
void serviceRelays() {
  int i;
  uint16_t oldState = state;
  uint16_t readState = 0;

  // Read the active pins  
  for (i=0; i<PIN_COUNT; i++) {
	uint16_t mask = (1 << i);
	if (mask & activePins) {
	  // lines go to ground when relays close. (closed = 1, open = 0)
	  if (LOW == digitalRead(i)) {
        readState |= mask;
      }
	}
  }

  // Add the latest sample to our ring buffer, giving us the last
  // n samples so we can debounce
  ringIndex = ++ringIndex % RING_SIZE;
  stateRing[ringIndex] = readState;

  // Debounce - each bit needs to be stable across all n samples
  // before we consider it valid.  
  uint16_t andBits = stateRing[0];  // andBits tells us what bits are stable at a value of 1
                                    // (each 1 bit in andBits indicates a stable value of 1)
  uint16_t orBits = stateRing[0];   // orBits will tell us what bits are stable at a value of 0
                                    // (each 0 bit in orBits indicate a stable value of 0)

  // combine our samples and see what's stable									
  for (i=1;i<RING_SIZE;i++) {
    andBits &= stateRing[i];
    orBits |= stateRing[i];
  }
                
  // Set bits that are stable at 1
  state |= andBits;

  // clear bits that are stable at 0
  state &= orBits;
  
  if (state != oldState) {
    time_t t = now();
    Serial.print(hour(t));  
    Serial.print(":");
    Serial.print(minute(t));
    Serial.print(":");
    Serial.println(second(t));
    Serial.print("Relay state: ");
    Serial.println(state);
  }

}

/*
 * Returns the current state in terms of I/O pins.
 * Used for debugging.
 */
uint16_t getRelayState() {
 
 return state; 
 
}

/*
 * Return the current state in terms of input lines.
 * For debouncing, we store the current state of the relays
 * by pin number.  However, since our callers want to be able to 
 * think in terms of the input lines on the ethernet jack, we need
 * to convert pin numbers to line number.
 * Lines are either on or off, so the returned value is a bitmask
 * with one bit per line.  Currently, only the first (low order)
 * seven are used.
  */
uint16_t getLineState() {

  int i;
  uint16_t lineState = 0;
  
  for (i=0; i<PIN_COUNT; i++) {
	if (pinToLineMap[i] >= 0) {   // value < 0 means line is not mapped to a pin
      uint16_t pinMask = (1 << i);
      uint16_t lineMask = (1 << pinToLineMap[i]);

	  if (pinMask & state) {   // If the pin is a 1
		lineState |= lineMask; // set the corresponding line to 1
	  }
	}
  }

  return lineState;
}
