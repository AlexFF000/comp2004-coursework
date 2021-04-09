/*
    Alex Redmond, Group R

    Header file for declarations related to reading the sensors
 */

// Struct for holding the readings from the sensors
 struct readings{
     float temperature;
     float pressure;
     float lightLevel;
 };

// Function to read the sensors and return a readings object
readings readSensors();
void initialiseSensors();
 