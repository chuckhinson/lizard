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

#define MAX_EVENTS (10)

void events_init();
void processRelayEvents(int);
event_t* getNextEvent(time_t, uint16_t);

#endif