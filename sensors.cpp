/*
    Alex Redmond, Group R
    Functions for reading data from sensors
*/
#include "mbed.h"
#include "sensors.h"
#include "BMP280_SPI.h"

AnalogIn ldr(PC_0);
BMP280_SPI environmentSensors(PB_5, PB_4, PB_3, PB_2);

void initialiseSensors(){
    // Initialise the environement sensors.  THIS WOULD BE BETTER IN A CLASS CONSTRUCTOR
    environmentSensors.initialize();
}

float readLDR(){
    // Read data from the LDR
    float data = ldr;
    return data;
}

float readPressure(){
    return environmentSensors.getPressure();
}

float readTemperature(){
    return environmentSensors.getTemperature();
}

readings readSensors(){
    // Read the sensors and return as a readings object
    readings data = {readTemperature(), readPressure(), readLDR()};
    return data;
}

