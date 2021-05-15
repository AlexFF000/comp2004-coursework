/*
    Alex Redmond, Group R
    Code largely based on example_code.cpp from COMP2004 example repository: https://github.com/UniversityOfPlymouthComputing/COMP2004-C1W2-2021/

    Header file containing definitions for the class for handling the SD card
*/
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
        SDBlockDevice sd{PB_5, PB_4, PB_3, PF_3};
        FATFileSystem fileSystem{"sd"};
};