#include "mbed.h"
#include "sensors.h"
#include "Buffer.h"

// main() runs in its own thread in the OS
void addItems();
void removeItems();
Thread t1, t2;
Buffer<readings> bf(2);
int main()
{
    t1.start(addItems);
    t2.start(removeItems);
    /*Buffer<readings> bf(2);
    Sensors sensors;
    while (true) {
        ThisThread::sleep_for(10s);
        readings env = sensors.readSensors();
        //printf("At %s, The temperature is %f degrees, the pressure is %f mbars, and the light level is %f\n", env.datetime.c_str(), env.temperature, env.pressure, env.lightLevel);
        bf.addItem(env);
    } */
}

void addItems(){
    while(true)
        bf.addItem(readings{"h", 1.1, 2.2, 3.3});
}

void removeItems(){
    readings *storage;
    while (true){
        bf.readItems(1, storage, true, true);
    }
}

