/*
    Alex Redmond, Group R
    Code largely based on example_code.cpp from COMP2004 example repository: https://github.com/UniversityOfPlymouthComputing/COMP2004-C1W2-2021/

    Header file containing definitions for the class for handling the SD card
*/
#ifndef SD_HEADER
#define SD_HEADER
#include "mbed.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"

#include "sensors.h"

class SDCard{
    public:
        SDCard();
        ~SDCard();
        void write(readings items[], int quantity);
        bool initialise();
        bool deInitialise();
        bool isInserted();
        bool initialised;
    private:
        /* 
            Pointers used for sd and fileSystem as deinit() and unmount() don't seem to fully deinitialise the objects (e.g. sd._isInitialized still seems to be true afterwards)
            This seems to cause issues with remounting the file system after the SD has been reinserted, even though unmount() and deinit() were called before removing
            To get around this, pointers are used so new instances can be created on each initialise() and the old ones destroyed on deInitialise()
        */
        SDBlockDevice *sd;
        FATFileSystem *fileSystem;
};
#endif