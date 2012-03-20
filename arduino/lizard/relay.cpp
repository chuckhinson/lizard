#include <Arduino.h>

#include "relay.h"

/*
 * The ethernet shield uses the SPI bus to communicate - this uses digital io pins 11,12, and 13.  It also 
 * uses pin 10 to select the ethernet chip and pin 4 to select the SD card (since there's no SD card on our 
 * ehternet shield, I'd think we should be able to use pin 4.  Pins 0 and 1 are the TX and RX pins for the
 * board's serial port, so we cant expect to use those if we still want to do serial debuggin.  Which leaves
 * us able to use pins 2 - 9
 *
 */
 
 
static int state = 0;

#define PIN_COUNT (16)
static uint16_t activePins = 0x03FC;    // 0000 0011 1111 1100

// There are 8 lines coming into the ethernet jack that we have connected to the relays.
// Since we want the rest of the system to think in terms of lines rather than I/O pins,
// we need to map the pins to their corresponding lines.  Note that line 8 is actually
// tied to ground, so we really only have 7 available lines coming in.
//                                                             1   1   1   1   1   1 
//                       pin   0   1  2  3  4  5  6  7  8  9   0   1   2   3   4   5
static int pinToLineMap[] = { -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, -1, -1, -1, -1, -1, -1};
 
// debounce sample ring buffer
#define RING_SIZE (3)
static uint16_t stateRing[ RING_SIZE ];
static uint16_t ringIndex = 0;

void relay_init() {
  int i;
  
  for (i=0; i<PIN_COUNT; i++) {
	uint16_t mask = (1 << i);
    Serial.print("checking pin ");
	if (mask & activePins) {
	  Serial.print("activating pin ");
	  Serial.println(i);
	  pinMode(i, INPUT);
      digitalWrite(i,HIGH);
	}
  }
  
}


void serviceRelay() {
  int i;
  uint16_t readState = 0;
  
  for (i=0; i<PIN_COUNT; i++) {
	uint16_t mask = (1 << i);
	if (mask & activePins) {
	  // lines go to ground when relays close. 1 == relay closed
	  if (LOW == digitalRead(i)) {
        readState |= mask;
      }
	}
  }

  
  ringIndex = ++ringIndex % RING_SIZE;
  stateRing[ringIndex] = readState;
  
  // This tells us what bits are stable at 1
  // 1 = stable at 1, 0 = unstable
  uint16_t andBits = stateRing[0];
  uint16_t orBits = stateRing[0];
  for (i=1;i<RING_SIZE;i++) {
    andBits &= stateRing[i];
    orBits |= stateRing[i];
  }
                
  // Set bits that are stable at 1
  state |= andBits;

  // clear bits that are stable at 0
  state &= orBits;
  
  
}

uint16_t getRelayState() {
 
 return state; 
}

uint16_t getLineState() {

  int i;
  uint16_t lineState = 0;
  
  for (i=0; i<PIN_COUNT; i++) {
	if (pinToLineMap[i] >= 0) {
      uint16_t pinMask = (1 << i);
      uint16_t lineMask = (1 << pinToLineMap[i]);

	  if (pinMask & state) {
		lineState |= lineMask;
	  }
	}
  }

  return lineState;
}
