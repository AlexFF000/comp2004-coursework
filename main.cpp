#include "mbed.h"
#include "sensors.h"
#include "Buffer.h"
#include <chrono>

// main() runs in its own thread in the OS
void addItems();
void removeItems();
void takeSample();
void wakeSampleThread();

Ticker samplingTicker;
Thread samplingThread, t2, t3, t4, t5, t6, t7;
Buffer<readings> samplesBuffer(50);
Sensors sensors;

chrono::milliseconds samplingInterval = 1s;  // Default sampling rate is once per second

int main()
{
    //while (true) printf("Alive");
    // Set up sampling thread
    samplingThread.start(takeSample);
    // Use ticker to repeatedly wake takeSample after samplingInterval
    samplingTicker.attach(&wakeSampleThread, samplingInterval);
}

void addItems(){
    printf("AddItems thread id is: %i", (int) ThisThread::get_id());
    float i = 0.0;
    while(true){
        //bf.addItem(readings{"h", 1.1, i, 3.3});
        i++;
        ThisThread::sleep_for(100ms);
    }
}

void removeItems(){
    printf("RemoveItems thread id is: %i", (int) ThisThread::get_id());
    readings storage[50];
    while (true){
        //bf.readItems(50, storage, false, true);
        printf("\nNew read\n");
        for (int i = 0; i < 50; i++)
            printf("Just read: %f", storage[i].pressure);
    }
}

void takeSample(){
    while(true){
        // Wait until woken by ticker interrupt
        uint32_t flags = ThisThread::flags_wait_all_for(1, 40s, true);   // Timeout is 40s as the longest sampling interval is 30s
        if (flags != 1){
            // CRITICAL ERROR (interrupt timeout, can this occur?)
        }
        // Take a sample and add it to buffer
        samplesBuffer.addItem(sensors.readSensors());
    }
}

void wakeSampleThread(){
    // A simple function to run in the ticker ISR to wake the sampling thread
    osSignalSet(samplingThread.get_id(), 1);
}

