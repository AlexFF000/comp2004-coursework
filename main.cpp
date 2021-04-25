#include "mbed.h"
#include "sensors.h"
#include "Buffer.h"

// main() runs in its own thread in the OS
int main()
{
    Buffer<readings> bf = Buffer<readings>(2);
    Sensors sensors = Sensors();
    while (true) {
        ThisThread::sleep_for(10s);
        readings env = sensors.readSensors();
        //printf("At %s, The temperature is %f degrees, the pressure is %f mbars, and the light level is %f\n", env.datetime.c_str(), env.temperature, env.pressure, env.lightLevel);
        bf.addItem(env);
    }
}

