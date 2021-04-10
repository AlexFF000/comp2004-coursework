/*
    Alex Redmond, Group R

    Header file for declarations related to reading the sensors
 */

// Struct for holding the readings from the sensors
 #include <string>
 using std::string;

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
 };
 