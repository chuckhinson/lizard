#include <Arduino.h>

#include "ntptime.h"
#include "events.h"

const int EVENT_RING_SZ = MAX_EVENTS;
static event_t event_buf[ EVENT_RING_SZ];
static int currEventIndex = 0;
static uint16_t prevRelayState = 0;

// Save the time the last event for each line was generated
static time_t lastEventTime[MAX_LINES];

// Save the state from the last event generate for each line
static int lastEventState[MAX_LINES];


time_t getLastEventTime(int i) {
  return lastEventTime[i];
}

int getLastEventState(int i) {
  return lastEventState[i];
}


/*
 * Create new events based on the current input line state.
 *
 * Note that we're using fake milliseconds in our event timestamps in 
 * order to give us unique timestamps.  Because the time returned by 
 * now() is not synchronized with the millisecond count returned by 
 * millis(), there's no easy way combine the two into a single timestamp 
 * with millisecond resolution. Instead, we will assume that we can never 
 * generate more than 1,000 events in a second and fabricate a millisecond 
 * value which is really a count of the number of events generated during 
 * the current second.
 */
void processRelayEvents(int newLineState) {

	event_t *currEvent = &event_buf[currEventIndex];

	time_t seconds = now();
	uint16_t milliseconds = 0;
	if (seconds == currEvent->seconds) {
		milliseconds = currEvent->millis;
	}
	
	uint16_t changes = newLineState ^ prevRelayState;  // find out which bits have change
	prevRelayState = newLineState;
	
	for(int i=0;i<MAX_LINES;i++) {
		uint16_t bitmask = ((uint16_t)1) << i;  // make a bitmask for the current bit
		if (changes & bitmask) {
			currEventIndex = (currEventIndex + 1) % EVENT_RING_SZ;
			currEvent = &event_buf[currEventIndex];
			currEvent->seconds = seconds;
			currEvent->millis = milliseconds++;
			currEvent->inputId = i; 
			currEvent->value = (newLineState & bitmask) ? 1 : 0;
			
			lastEventTime[i] = currEvent->seconds;
			lastEventState[i] = currEvent->value;
			
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

		if ((event_buf[index].seconds > seconds) ||
			((event_buf[index].seconds == seconds) && (event_buf[index].millis > milliseconds))){
			nextEvent = &event_buf[index];
			break;
		}
	}

	return nextEvent;

}

void initEvents() {

	int i=0;
	
	for (i=0;i<MAX_LINES;i++) {
		lastEventTime[i] = getBootTime();
	}

}


