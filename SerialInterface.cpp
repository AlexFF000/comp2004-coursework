/*
    Alex Redmond, Group R
    
    Function definitions for SerialInterface
*/
#include <string>
#include "SerialInterface.h"

UnbufferedSerial serial(USBTX, USBRX);
DigitalOut redLed(PC_2);

// Buffer for serial input
char commandBuffer[32];
int commandBufferIndex = 0;
bool serialDisabled;  // Set to true while a command is being processed to prevent further input

void commandISR();

SerialInterface *SerialInterface::instance = NULL;

SerialInterface::SerialInterface(EventQueue *eventQueue){
    this->eventQueue = eventQueue;
    instance = this;
    serial.attach(&commandISR);
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

void processCommand(){
    string command(commandBuffer);
    if (command.find("READ NOW") == 0){
        SerialInterface::log("\nYou said READ NOW");
    }
    else if (command.find("READBUFFER") == 0){
        SerialInterface::log("\nYou said READBUFFER");
    }
    else if (command.find("STATE") == 0){
        SerialInterface::log("\nYou said STATE");
    }
    else if (command.find("LOGGING") == 0){
        SerialInterface::log("\nYou said LOGGING");
    }
    else if (command.find("SD") == 0){
        SerialInterface::log("\nYou said SD");
    }
    else {
        SerialInterface::log("\nUnrecongnised Command");
    }
    // Clear commandBuffer
    for (int i = 0; i < 32; i++){
        commandBuffer[i] = 0;
    }
    commandBufferIndex = 0;
    serialDisabled = false;
}

void commandISR(){
    char c;
    if (serial.read(&c, 1)){
        // Echo back to serial and add to commandBuffer
        if (!serialDisabled){
            // If a command is already being processed, ignore any further input to avoid race conditions on serialBuffer
            if (c == 13){
                // Ignore further serial input until this command has been processed
                serialDisabled = true;
                SerialInterface::instance->eventQueue->call(&processCommand);
            }
            else if (c == 8){
                // Go back a char
                commandBufferIndex = 0 <= commandBufferIndex - 1 ? commandBufferIndex -1 : 0;
            }
            else{
                commandBuffer[commandBufferIndex] = c;
                commandBufferIndex++;
                if (commandBufferIndex == 32){
                    // Flip back to 0 to prevent buffer overflow
                    commandBufferIndex = 0;
                }
                serial.write(&c, 1);
            }
        }
        // SerialInterface::log("Interrupt");
    }
}