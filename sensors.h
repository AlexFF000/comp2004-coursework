/*
    Alex Redmond, Group R

    Header file for declarations related to reading the sensors
 */

#include "mbed.h"
#include <string>
#include "BMP280_SPI.h"
using std::string;

// Struct for holding the readings from the sensors
struct readings{
     string datetime;
     float temperature;
     float pressure;
     float lightLevel;
 };

 // Class containing methods for getting readings from sensors (class used so initialisation can be done in constructor)
 class Sensors{
    public:
        Sensors();
        readings readSensors();
    private:
        float readLDR(), readTemperature(), readPressure();
        AnalogIn ldr{PC_0};
        BMP280_SPI environmentSensors{PB_5, PB_4, PB_3, PB_2};
 };
 