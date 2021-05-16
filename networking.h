/*
    Alex Redmond, Group R
    Header file with declarations for networking related functions
*/
#include "Buffer.h"
#include "sensors.h"
// Start the web server for displaying most recent samples
int setupEthernet();
int runServer(Buffer<readings> *samplesBuffer);
// Get the time from an NTP server
time_t getTime();