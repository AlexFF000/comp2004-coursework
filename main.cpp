#include "mbed.h"
#include <chrono>
#include <ctime>

#include "sensors.h"
#include "Buffer.h"
#include "SDCard.h"
#include "networking.h"
#include "SerialInterface.h"
#include "globals.h"


// main() runs in its own thread in the OS
void addItems();
void removeItems();
void takeSample();
void wakeSampleThread();
void writeItemsToSD();

void startWebServer();
void handleUserButton();
void userButtonISR();


EventQueue mainQueue;  // Event queue for main thread
EventFlags sampleFlags;
Ticker samplingTicker;
Thread samplingThread, sdThread, httpThread, t4, t5, t6, t7;
Buffer<readings> samplesBuffer(700);
Sensors sensors;
SDCard sd;
InterruptIn userBtn(USER_BUTTON);
SerialInterface terminal(&mainQueue);

time_t samplingInterval = 1000;  // Default sampling rate is once per second
int sdWriteThreshold = 159;

bool userButtonDisabled;  // Set to true upon the user button being pressed, and set to false again once the press has been fully handled.  (To avoid noise causing handler to run multiple times)

/*
    The space for holding readings to write to SD card after they have been fetched from the buffer.
    This must be allocated on heap as the maximum number of readings is 700, which could cause a stack overflow
    It is allocated once up here (rather than being allocated and deallocated on each write to SD) to avoid memory fragmentation
*/
readings *sdWriteBuffer = new readings[700];

int main()
{
    userBtn.rise(userButtonISR);
    //while (true) printf("Alive");
    //ThisThread::sleep_for(1s);
    //SDCard sd;
    // Set up sampling thread
    samplingThread.start(takeSample);
    // Use ticker to repeatedly wake takeSample after samplingInterval
    changeSamplingInterval(samplingInterval);
    sdThread.start(&writeItemsToSD);
    if (setupEthernet() == 0){
        SerialInterface::log("Successfully connected to network");
        httpThread.start(&startWebServer);
        time_t timeNow = getTime();
        printf("Time is %u", timeNow);
        set_time(timeNow);
    }
    else{
        // LOG ERROR (NOT CRITICAL)
        printf("\nFailed to setup Ethernet interface.  Time will be wrong and HTTP server will be unavailable\n");
        SerialInterface::log("\nThis was sent using logger.  Failed to setup Ethernet interface\n");
    }
    
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


void writeItemsToSD(){
    printf("Write thread is %i", ThisThread::get_id());
    while (true){
        if (sampleFlags.get() == 1){
            // Only write to sd if sampleFlag 1 is set
            // readings samples[sdWriteThreshold];
            samplesBuffer.readItems(sdWriteThreshold, sdWriteBuffer, false, true);
            SerialInterface::log("I have been woken, and am about to write");
            sd.write(sdWriteBuffer, sdWriteThreshold);
        }
        else{
            sampleFlags.wait_all(1, false);
        }
        
    }
}

void takeSample(){
    printf("Sample thread is %i", ThisThread::get_id());
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

void changeSamplingInterval(time_t interval, bool changeInterval){
    // Changes the interval between samples.  If interval is 0, disables sampling entirely.  If changeInterval is false, it will just re-enable sampling with the existing interval
    if (0 < interval && changeInterval){
        // Calculate number of items for SD write
        // This calculation roughly meets the criteria that SD writes are done once per minute at the most frequent, and once an hour at the least
        // It adds roughly 11s to the interval between SD writes for every 100ms sampling interval, but starts from 60s
        sdWriteThreshold = round(((((interval / 100) - 1) * 11000) + 60000) / interval);
        samplingInterval = interval;
    }
    // Clear existing ticker
    samplingTicker.detach();
    if (0 < interval){
        // Make sure flags for writing to SD are set
        sampleFlags.set(1);
        samplingTicker.attach(&wakeSampleThread, std::chrono::milliseconds(samplingInterval));
    }
    else sampleFlags.clear();
}

void startWebServer(){
    // Start web server for serving most recent sample
    runServer(&samplesBuffer);
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

