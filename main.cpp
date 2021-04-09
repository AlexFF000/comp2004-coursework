#include "mbed.h"
#include "sensors.h"

// main() runs in its own thread in the OS
int main()
{
    initialiseSensors();
    while (true) {
        ThisThread::sleep_for(10s);
        readings env = readSensors();
        printf("The temperature is %f degrees, the pressure is %f mbars, and the light level is %f\n", env.temperature, env.pressure, env.lightLevel);
    }
}

