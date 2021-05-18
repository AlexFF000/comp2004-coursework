/*
    Alex Redmond, Group R
    
    Class for handling I/O through serial.  Partially uses singleton pattern
*/
#include "mbed.h"
#include "Buffer.h"
#include "sensors.h"

extern Buffer<readings> samplesBuffer;

class SerialInterface{
    public:
        SerialInterface(EventQueue *eventQueue);
        static void log(char *text);
        static void criticalError(char *text);

        static SerialInterface *instance;
        EventQueue *eventQueue;
};