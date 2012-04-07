#ifndef network_h
#define network_h 

void initNetwork();
void serviceNetwork();

extern time_t lastNetworkRestartTime;
extern int networkRestartCount;


#endif