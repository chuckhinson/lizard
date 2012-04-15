#ifndef ntptime_h
#define ntptime_h 

#include<time.h>

void initNtpTime();
void restartNtpTime();
void serviceNtpTime();
time_t getBootTime();

#endif
