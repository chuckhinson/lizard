
#ifndef wserver_h
#define wserver_h

void initWebServer();
void serviceWebServer();
time_t getLastRequestTime();
int getRequestCount();

#endif
