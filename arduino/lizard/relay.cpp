#include <Arduino.h>

#include "relay.h"

/*
 * The ethernet shield uses the SPI bus to communicate - this uses digital io pins 11,12, and 13.  It also 
 * uses pin 10 to select the ethernet chip and pin 4 to select the SD card (since there's no SD card on our 
 * ehternet shield, I'd think we should be able to use pin 4.
 *
 */
 
 
static int state = 0;

#define LOW_PORT (4)
#define HIGH_PORT (9)
 
// debounce sample ring buffer
#define RING_SIZE (3)
static uint16_t stateRing[ RING_SIZE ];
static uint16_t ringIndex = 0;

void relay_init() {
  int i;
  
  for (i=LOW_PORT; i<=HIGH_PORT; i++) {
    pinMode(i, INPUT);
    digitalWrite(i,HIGH);
  }
  
  
}


void serviceRelay() {
  int i;
  uint16_t readState = 0;
  
  for (i=LOW_PORT; i<=HIGH_PORT; i++) {
    if (LOW == digitalRead(i)) {
      readState |= (1 << (i-LOW_PORT));
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
