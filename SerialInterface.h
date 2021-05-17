/*
    Alex Redmond, Group R
    
    Class for handling I/O through serial.  Partially uses singleton pattern
*/
#include "mbed.h"

class SerialInterface{
    public:
        SerialInterface(EventQueue *eventQueue);
        static void log(char *text);
        static void criticalError(char *text);

        static SerialInterface *instance;
        EventQueue *eventQueue;
};