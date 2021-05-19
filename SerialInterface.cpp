/*
    Alex Redmond, Group R
    
    Function definitions for SerialInterface
*/
#include <cstdio>
#include <string>
#include "globals.h"
#include "SerialInterface.h"

UnbufferedSerial serial(USBTX, USBRX);
DigitalOut redLed(PC_2);

// Buffer for serial input
char commandBuffer[32];
int commandBufferIndex = 0;
bool serialDisabled;  // Set to true while a command is being processed to prevent further input

void serialWriteWrapper(char *text);
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
        if (instance->logging == true) instance->eventQueue->call(serialWriteWrapper, text);
    }
}

void SerialInterface::criticalError(char *text){
    // Turn on redLed
    redLed = 1;
    // Print regardless of whether logging is enabled or not
    serialWriteWrapper(text);
    // Give the user time to read the output
    ThisThread::sleep_for(5s);
    // Restart
    NVIC_SystemReset();
}

void serialWriteWrapper(char *text){
    // Add newline to text and send to serial.out
    std::string textNewLine = std::string(text);
    textNewLine += "\n";
    serial.write(textNewLine.c_str(), textNewLine.length());
    serial.sync();
}

void processCommand(){
    string command(commandBuffer);
    if (command.find("READ NOW") == 0){
        readings latest;
        if (samplesBuffer.readLastN(1, &latest) == 1){
            char datetime[20];
            strftime(datetime, 19, "%F %T", localtime(&latest.datetime));
            char item[100];
            sprintf(item, "\n%s: temp: %3.3f, pressure: %4.3f, light: %3.3f", datetime, latest.temperature, latest.pressure, latest.lightLevel);
            serialWriteWrapper(item);
        }
        else{
            serialWriteWrapper("\nBuffer is empty");
        }
    }
    else if (command.find("READBUFFER") == 0){
        // Command is in format READBUFFER n, with n being number of samples to read from buffer
        // Get number of items to read
        int quantity = 0;
        bool readAll = false;
        if (commandBuffer[11] == 45){
            // If number is negative, read all entries
            readAll = true;
        }
        else{
            for (int i = 11; i < 15; i++){
                // Read up to 4 digits
                if (48 <= commandBuffer[i] && commandBuffer[i] <= 57){
                    // If it is a digit.  Shift existing value one place left and add new digit in ones column
                    quantity *= 10;
                    quantity += (commandBuffer[i] - 48);
                }
                else{
                    if (i == 11){
                        // The first digit is invalid so return error
                        serialWriteWrapper("\nInvalid number provided");
                        // Clear commandBuffer
                        for (int i = 0; i < 32; i++){
                            commandBuffer[i] = 0;
                        }
                        commandBufferIndex = 0;
                        serialDisabled = false;
                        return;
                    }
                    break;
                }
            }
        }
        if (readAll){
            // Use the flush method to get all entries, but do not actually clear the buffer
            ArrayWithLength<readings> samples = samplesBuffer.flush(false);
            char datetime[20];
            char data[100];
            sprintf(data, "\nReading all %i items", samples.length);
            serialWriteWrapper(data);
            for (int i = samples.length - 1; 0 <= i; i--){
                strftime(datetime, 19, "%F %T", localtime(&samples.items[i].datetime));
                sprintf(data, "%s: temp: %3.3f, pressure: %4.3f, light: %3.3f", datetime, samples.items[i].temperature, samples.items[i].pressure, samples.items[i].lightLevel);
                serialWriteWrapper(data);
            }
        }
        else{
            readings samples[quantity];
            char datetime[20];
            char data[100];
            int itemsRead = samplesBuffer.readLastN(quantity, samples);
            sprintf(data, "\nReading %i items", itemsRead);
            serialWriteWrapper(data);
            for (int i = 0; i < itemsRead; i++){
                strftime(datetime, 19, "%F %T", localtime(&samples[i].datetime));
                sprintf(data, "%s: temp: %3.3f, pressure: %4.3f, light: %3.3f", datetime, samples[i].temperature, samples[i].pressure, samples[i].lightLevel);
                serialWriteWrapper(data);
            }
        }
    }
    else if (command.find("SET T") == 0){
        // Command is in format SET T n, where n is the number of seconds between samples (can be fractional)
        time_t interval = 0;
        // Read up to two digits before the decimal point, and 1 after
        int i = 6;
        while (i < 8){
            if (48 <= commandBuffer[i] && commandBuffer[i] <= 57){
                // If it is a digit.  Shift existing value one place left and add new digit in ones column
                interval *= 10;
                interval += (commandBuffer[i] - 48) * 1000;  // 1000ms in a second
                i++;
            }
            else{
                if (i == 6){
                    // The first digit is not a digit
                    serialWriteWrapper("\nInvalid Number");
                    // Clear commandBuffer
                    for (int i = 0; i < 32; i++){
                        commandBuffer[i] = 0;
                    }
                    commandBufferIndex = 0;
                    serialDisabled = false;
                    return;
                }
                break;
            }
        }
        if (commandBuffer[i] == 46){
            // A decimal point
            if (48 <= commandBuffer[i + 1] && commandBuffer[i + 1] <= 57){
                interval += (commandBuffer[i + 1] - 48) * 100;
            }
        }
        
        if (100 <= interval && interval <= 30000){
            changeSamplingInterval(interval);
            char text[50];
            sprintf(text, "\nT updated to %ims", interval);
            serialWriteWrapper(text);
        }
        else{
            serialWriteWrapper("\nOut of Range: T must be between 0.1 and 30");
        }
    }
    else if (command.find("STATE") == 0){
        if (commandBuffer[6] == 79 && commandBuffer[7] == 78){
            // Turn on sampling
            changeSamplingInterval(1, false);
            serialWriteWrapper("\nTurned sampling on");
        }
        else if (commandBuffer[6] == 79 && commandBuffer[7] == 70 && commandBuffer[8] == 70){
            // Turn off sampling
            changeSamplingInterval(0);
            serialWriteWrapper("\nTurned sampling off");
        }
        else{
            serialWriteWrapper("\nInvalid value, expected ON or OFF");
        }
    }
    else if (command.find("LOGGING") == 0){
        if (commandBuffer[8] == 79 && commandBuffer[9] == 78){
            // Turn on logging
            SerialInterface::instance->logging = true;
            serialWriteWrapper("\nTurned logging on");
        }
        else if (commandBuffer[8] == 79 && commandBuffer[9] == 70 && commandBuffer[10] == 70){
            // Turn off logging
            SerialInterface::instance->logging = false;
            serialWriteWrapper("\nTurned logging off");
        }
        else{
            serialWriteWrapper("\nInvalid value, expected ON or OFF");
        }
    }
    else if (command.find("SD") == 0){
        if (commandBuffer[3] == 70){
            // Flush buffer to SD card
            ArrayWithLength<readings> items = samplesBuffer.flush();
            sd.write(items.items, items.length);
        }
        else if (commandBuffer[3] == 69){
            // Flush buffer and unmount SD card
            ArrayWithLength<readings> items = samplesBuffer.flush();
            sd.write(items.items, items.length);
            sd.deInitialise();
        }
        else{
            serialWriteWrapper("\nInvalid value, expected F or E");
        }
    }
    else {
        serialWriteWrapper("\nUnrecognised Command");
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
                serial.write(&c, 1);
            }
            else{
                // Add to commandBuffer and echo back to serial so user knows what they are typing
                // Convert lower case to upper case to make case insensitive
                if (97 <= c && c <= 122){
                    c-= 32;
                }
                commandBuffer[commandBufferIndex] = c;
                commandBufferIndex++;
                if (commandBufferIndex == 32){
                    // Flip back to 0 to prevent buffer overflow
                    commandBufferIndex = 0;
                }
                serial.write(&c, 1);
                
            }
        }
    }
}