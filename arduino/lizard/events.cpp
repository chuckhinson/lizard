#include <Arduino.h>

#include "events.h"

#define EVENT_RING_SZ MAX_EVENTS

static event_t event_buf[ EVENT_RING_SZ];
static int currEventIndex = 0;
static uint16_t prevRelayState = 0;

void events_init() {

}



/*
 * Create new events based on the current relay state.
 *
 * Note that we're using fake milliseconds in our event timestampsin 
 * order to give us unique timestamps.  Because the time returned by 
 * now() is not synchronized with the millisecond count returned by 
 * millis(), there's no easy way combine the two into a single timestamp 
 * with millisecond resolution. Instead, we will assume that we can never 
 * generate more than 1,000 events in a second and fabricate a millisecond 
 * value which is really a count of the number of events generated during 
 * the current second.
 */
void processRelayEvents(int newRelayState) {

	event_t *currEvent = &event_buf[currEventIndex];

	time_t seconds = now();
	uint16_t milliseconds = 0;
	if (seconds == currEvent->seconds) {
		milliseconds = currEvent->millis;
	}
	
	uint16_t changes = newRelayState ^ prevRelayState;  // find out which bits have change
	prevRelayState = newRelayState;
	
	for(int i=0;i<sizeof(changes);i++) {
		uint16_t bitmask = ((uint16_t)1) << i;  // make a bitmask for the current bit
		if (changes & bitmask) {
			currEventIndex = (currEventIndex + 1) % EVENT_RING_SZ;
			currEvent = &event_buf[currEventIndex];
			currEvent->seconds = seconds;
			currEvent->millis = milliseconds++;
			currEvent->inputId = i+1; // pin numbers start at 1
			currEvent->value = (newRelayState & bitmask) ? 1 : 0;
			Serial.print("id: ");
			Serial.print(currEvent->inputId);
			Serial.print("  value: ");
			Serial.println(currEvent->value);
		}
	}
	
	
}


/*
 * Get the next event that occurred after the time specified
 * by seonds and milliseconds.
 */
event_t* getNextEvent(time_t seconds, uint16_t milliseconds) {

	event_t *nextEvent = 0;

	for (int i=1; i<= EVENT_RING_SZ; i++) {
		int index = (currEventIndex + i) % EVENT_RING_SZ;

		Serial.print("event:");
		Serial.print(event_buf[index].seconds);
		Serial.print(":");
		Serial.println(event_buf[index].millis);
		if ((event_buf[index].seconds > seconds) ||
			((event_buf[index].seconds == seconds) && (event_buf[index].millis > milliseconds))){
			nextEvent = &event_buf[index];
			break;
		}
	}

	return nextEvent;

}
