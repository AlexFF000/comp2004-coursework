#include "mbed.h"
#include "sensors.h"
#include "Buffer.h"

// main() runs in its own thread in the OS
void addItems();
void removeItems();
Thread t1, t2;
Buffer<readings> bf(600);
int main()
{
    printf("-----------New Start-----------");
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
    printf("AddItems thread id is: %i", (int) ThisThread::get_id());
    while(true){
        bf.addItem(readings{"h", 1.1, 2.2, 3.3});
        ThisThread::sleep_for(100ms);
    }
}

void removeItems(){
    printf("RemoveItems thread id is: %i", (int) ThisThread::get_id());
    readings storage[2];
    while (true){
        bf.readItems(2, storage, true, true);
        printf("Just read: %f", storage[0].pressure);
    }
}

