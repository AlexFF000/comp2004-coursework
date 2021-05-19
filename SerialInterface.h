/*
    Alex Redmond, Group R
    
    Class for handling I/O through serial.  Partially uses singleton pattern
*/
#ifndef SERIAL_HEADER
#define SERIAL_HEADER
#include "mbed.h"
#include "sensors.h"


extern Ticker samplingTicker;

class SerialInterface{
    public:
        SerialInterface(EventQueue *eventQueue);
        static void log(char *text);
        static void criticalError(char *text);

        bool logging = true;  // Turn logging on and off (critical errors not affected)
        static SerialInterface *instance;
        EventQueue *eventQueue;
};
#endif