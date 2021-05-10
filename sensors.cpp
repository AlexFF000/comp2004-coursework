/*
    Alex Redmond, Group R

    Functions for reading data from sensors
*/
#include "mbed.h"
#include "sensors.h"
#include "BMP280_SPI.h"

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
    // Read the sensors and return as a readings object, including the current time in ctime format (Www Mmm dd hh:mm:ss yyyy)
    time_t timestamp = time(NULL);
    return readings {ctime(&timestamp), readTemperature(), readPressure(), readLDR()};
}

