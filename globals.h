/*
    Alex Redmond, Group R

    Header file for variables and functions that should be usable across multiple files
*/
#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER
#include "time.h"
#include "Buffer.h"
#include "sensors.h"
#include "SDCard.h"
// Start the sampling thread and the writing to SD thread
void changeSamplingInterval(time_t interval, bool changeInterval=true);

extern Buffer<readings> samplesBuffer;
extern EventFlags sampleFlags;  // Used to notify the writing to SD thread if logging is on or off
extern SDCard sd;
#endif