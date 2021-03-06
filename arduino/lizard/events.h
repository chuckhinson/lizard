#ifndef EVENTS_H
#define EVENTS_H

#include <inttypes.h>
#include <time.h>

typedef struct {
	uint32_t	seconds;
	uint16_t	millis;
	uint8_t		inputId;
	uint8_t		value;
} event_t;	

// Controls the size of the event history buffer; e.g., we store
// store the last n events wher n=MAX_EVENTS.  Be careful when 
// adjusting this value.  Not only does it affect the size of 
// statically allocated data space, it will also affect the amount 
// of stack space needed.
#define MAX_EVENTS (16)

// Although there are physically 8 lines coming in to the box, only 7 
// are for input.  The eighth is for ground, and is thus not counted as a line
#define MAX_LINES (7)


void initEvents();
void processRelayEvents(int);
event_t* getNextEvent(time_t, uint16_t);
time_t getLastEventTime(int);
int getLastEventState(int);

#endif