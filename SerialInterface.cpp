/*
    Alex Redmond, Group R
    
    Function definitions for SerialInterface
*/
#include "SerialInterface.h"

DigitalOut redLed(PC_2);

SerialInterface *SerialInterface::instance = NULL;

SerialInterface::SerialInterface(EventQueue *eventQueue){
    this->eventQueue = eventQueue;
    instance = this;
}

void SerialInterface::log(char *text){
    if (instance != NULL){
        // Can only log if the class has been instantiated 
        instance->eventQueue->call(printf, "%s\n", text);
    }
}

void SerialInterface::criticalError(char *text){
    // Turn on redLed
    redLed = 1;
    // Print regardless of whether logging is enabled or not
    printf("%s\n", text);
    // Give the user time to read the output
    ThisThread::sleep_for(5s);
    // Restart
    NVIC_SystemReset();
}