#ifndef network_h
#define network_h 

void initNetwork();
void serviceNetwork();
void startEthernet();

extern time_t lastNetworkRestartTime;
extern int networkRestartCount;


#endif