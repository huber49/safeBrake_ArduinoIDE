#ifndef ReadSensors_h
#define ReadSensors_h

#include "Arduino.h"
#include "constants.h"

// Read Values at ADCs and converts into Volt.

float readVccSense();

float readGndSense();

float readSwitchSense1();

float readSwitchSense2();

int readIgnitionSense();

int readThrottleSense();

#endif
