/*
    Alex Redmond, Group R
    
    Functions for reading data from sensors
*/
#include "mbed.h"
#include "sensors.h"
#include "BMP280_SPI.h"

AnalogIn ldr(PC_0);
BMP280_SPI environmentSensors(PB_5, PB_4, PB_3, PB_2);

Sensors::Sensors(){
    environmentSensors.initialize();
}


float Sensors::readLDR(){
    // Read data from the LDR
    float data = ldr;
    return data;
}

float Sensors::readPressure(){
    return environmentSensors.getPressure();
}

float Sensors::readTemperature(){
    return environmentSensors.getTemperature();
}

readings Sensors::readSensors(){
    // Read the sensors and return as a readings object
    readings data = {readTemperature(), readPressure(), readLDR()};
    return data;
}

