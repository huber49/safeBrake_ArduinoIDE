#ifndef ReadSensors_h
#define ReadSensors_h

#include "Arduino.h"
#include "constants.h"

// Read Values at ADCs and converts into Volt.

void getSensorValues(unsigned int sensorValues[]);

unsigned int readIgnitionSense(); //Sonderfall

#endif
