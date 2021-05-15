/*
    Alex Redmond, Group R
    Code largely based on example_code.cpp from COMP2004 example repository: https://github.com/UniversityOfPlymouthComputing/COMP2004-C1W2-2021/

    Method definitions for SDCard class
*/

#include "SDCard.h"
#include "sensors.h"
#include "mbed.h"
#include <cstdio>
#include <ctime>

Thread blinkLed;
DigitalIn cardDetect(PF_4, PullUp);
DigitalOut greenLed(PC_6);

void flashGreenLed(){
    while (true){
        greenLed = !greenLed;
        ThisThread::sleep_for(250ms);
    }
}
void startFlashingGreenLed(){
    blinkLed.start(&flashGreenLed);
}

void stopFlashingGreenLed(){
    blinkLed.terminate();
}

SDCard::SDCard(){
    printf("\nHello from the SD card!\n");
    if (this->isInserted()){
        // Try to initialise card
        printf("\nInserted\n");
        this->initialise();
    }
    else{
        // SD card not inserted
        printf("\nNot Inserted\n");
    }
}

SDCard::~SDCard(){
    // Deinitialise
    this->deInitialise();
}

bool SDCard::initialise(){
    if (this->isInserted()){
        startFlashingGreenLed();
        // Mount the sd card
        if (this->sd.init() == 0){
            // Initialised successfully, mount file system
            if (this->fileSystem.mount(&sd) == 0){
                this->initialised = true;
                printf("\nSuccessful init\n");
                stopFlashingGreenLed();
                greenLed = 1;  // Turn on green LED when SD card is mounted
                return true;
            }
        }
        // Otherwise initialisation failed
        printf("\nFailed to init SD\n");
        return false;
    }
    else{
        // Card not inserted
        return false;
    }
}

bool SDCard::deInitialise(){
    if (this->initialised){
        // Unmount file system and de-initialise SDBlockDevice
        startFlashingGreenLed();
        if (this->fileSystem.unmount() == 0){
            if (this->sd.deinit() == 0){
                stopFlashingGreenLed();
                this->initialised = false;
                return true;
            }
        }
        return false;
    }
    // Not initalised so no need to deinit
    return true;
}

bool SDCard::isInserted(){
    // The card detect pin is connected to ground, except when a card is inserted.  Therefore it returns 1 if no card is inserted and 0 if it is, so must be flipped
    return !cardDetect;
}

void SDCard::write(readings *items, int quantity){
    if (this->initialised){
        // Flash green led while writing
        startFlashingGreenLed();
        FILE *fp = fopen("/sd/data.txt", "a");
        if (fp != NULL){
            char datetime[19];
            for (int i = 0; i < quantity; i++){
                // Write each entry to the SD card in format: <Date/Time>: temp: <temp>, pressure: <pressure>, light: <light level>
                // strftime used because ctime adds unwanted newline
                strftime(datetime, 19, "%F %T", localtime(&items[i].datetime));
                fprintf(fp, "%s: temp: %f, pressure: %f, light: %f\n", datetime, items[i].temperature, items[i].pressure, items[i].lightLevel);
            }
            fclose(fp);
            stopFlashingGreenLed();
            greenLed = 1;
            // LOG
            printf("Wrote data");
        }
        else{
            // Handle SD card full
            // Error: Cannot open file
            printf("Was trying to write but something went wrong!");
        }

    }
}
