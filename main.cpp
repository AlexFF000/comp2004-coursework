#include "mbed.h"
#include "sensors.h"
#include "Buffer.h"
#include "SDCard.h"
#include <chrono>

// main() runs in its own thread in the OS
void addItems();
void removeItems();
void takeSample();
void wakeSampleThread();
void writeItemsToSD();

void handleUserButton();
void userButtonISR();


EventQueue mainQueue;  // Event queue for main thread
Ticker samplingTicker;
Thread samplingThread, sdThread, t3, t4, t5, t6, t7;
Buffer<readings> samplesBuffer(50);
Sensors sensors;
SDCard sd;
InterruptIn userBtn(USER_BUTTON);

chrono::milliseconds samplingInterval = 1s;  // Default sampling rate is once per second

bool userButtonDisabled;  // Set to true upon the user button being pressed, and set to false again once the press has been fully handled.  (To avoid noise causing handler to run multiple times)

int main()
{
    userBtn.rise(userButtonISR);
    //while (true) printf("Alive");
    //ThisThread::sleep_for(1s);
    //SDCard sd;
    // Set up sampling thread
    samplingThread.start(takeSample);
    // Use ticker to repeatedly wake takeSample after samplingInterval
    samplingTicker.attach(&wakeSampleThread, samplingInterval);
    sdThread.start(&writeItemsToSD);
    //while(true)printf("SD card is inserted?: %i", sd.isInserted());
    mainQueue.dispatch_forever();
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

void writeItemsToSD(){
    while (true){
        readings samples[50];
        samplesBuffer.readItems(50, samples, false, true);
        sd.write(samples, 50);
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

void handleUserButton(){
    // Handles user button in a thread rather than ISR
    printf("Handler started");
    if (sd.initialised){
        // If the SD card is already initialised, then the user button flushes the buffer and unmounts it
        ArrayWithLength<readings> items = samplesBuffer.flush();
        sd.write(items.items, items.length);
        sd.deInitialise();
    }
    else{
        // If the SD card is not initialised, the user button mounts it and flushes the buffer to it
        sd.initialise();
        ArrayWithLength<readings> items = samplesBuffer.flush();
        sd.write(items.items, items.length);
    }
    // Wait 500ms before re-enabling the button in case of bounce
    ThisThread::sleep_for(500ms);
    userButtonDisabled = false;
}

void userButtonISR(){
    // Lock on userButtonDisable is unnecessary, as another button press interrupt will have same priority as this so won't interrupt it
    if (userButtonDisabled == false){
        userButtonDisabled = true;
        mainQueue.call(&handleUserButton);
    }
}

