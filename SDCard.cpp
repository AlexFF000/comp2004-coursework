/*
    Alex Redmond, Group R
    Code largely based on example_code.cpp from COMP2004 example repository: https://github.com/UniversityOfPlymouthComputing/COMP2004-C1W2-2021/

    Method definitions for SDCard class
*/
#include "SerialInterface.h"
#include "SDCard.h"
#include "sensors.h"
#include "mbed.h"
#include <cstdio>
#include <ctime>

Thread blinkLed;
DigitalIn cardDetect(PF_4, PullUp);
DigitalOut greenLed(PC_6);
EventFlags blinkFlags;


void flashGreenLed(){
    // EventFlags was used for this rather than Thread flags, are thread flags wouldn't seem to clear
    while (true){
        if (blinkFlags.get() == 1){
            greenLed = !greenLed;
            ThisThread::sleep_for(250ms);
        }
        else{
            blinkFlags.wait_all(1, false);
        }
    }
}
void startFlashingGreenLed(){
    blinkFlags.set(1);
}

void stopFlashingGreenLed(){
    blinkFlags.clear();
}

SDCard::SDCard(){
    printf("\nHello from the SD card!\n");
    blinkLed.start(&flashGreenLed);
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
        // sd and fileSystem objects recreated on each initialise to get around issue explained in SDCard.h
        this->sd = new SDBlockDevice(PB_5, PB_4, PB_3, PF_3);
        this->fileSystem = new FATFileSystem("sd");
        if (this->sd->init() == 0){
            // Initialised successfully, mount file system
            int result = this->fileSystem->mount(sd);
            if (result == 0){
                this->initialised = true;
                printf("\nSuccessful init\n");
                stopFlashingGreenLed();
                greenLed = 1;  // Turn on green LED when SD card is mounted
                return true;
            }
            printf("error: %i", result);
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
        printf("Unmounting");
        startFlashingGreenLed();
        if (this->fileSystem->unmount() == 0){
            // sd and fileSystem objects deleted on each deInitialise to get around issue explained in SDCard.h
            delete this->fileSystem;
            if (this->sd->deinit() == 0){
                delete this->sd;
                stopFlashingGreenLed();
                greenLed = 0;
                this->initialised = false;
                printf("UnmountED");
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
        char str[30];
        sprintf(str, "Writing %i items", quantity);
        SerialInterface::log(str);
        startFlashingGreenLed();
        FILE *fp = fopen("/sd/data.txt", "a");
        if (fp != NULL){
            char datetime[20];
            for (int i = 0; i < quantity; i++){
                // Write each entry to the SD card in format: <Date/Time>: temp: <temp>, pressure: <pressure>, light: <light level>
                // strftime used because ctime adds unwanted newline
                strftime(datetime, 20, "%F %T", localtime(&items[i].datetime));
                if (fprintf(fp, "%s: temp: %f, pressure: %f, light: %f\n", datetime, items[i].temperature, items[i].pressure, items[i].lightLevel) < 0){
                    // Log error
                    printf("Unable to write");
                    break;
                }
            }
            fclose(fp);
            stopFlashingGreenLed();
            greenLed = 1;
            // LOG
            printf("Wrote data");
        }
        else{
            stopFlashingGreenLed();
            greenLed = 1;
            // Handle SD card full
            // Error: Cannot open file
            printf("Was trying to write but something went wrong!");
        }

    }
}
