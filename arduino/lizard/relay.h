#ifndef RELAY_H
#define RELAY_H

#include <inttypes.h>

void initRelays();
void serviceRelays();
uint16_t getRelayState();
uint16_t getLineState();

#endif
