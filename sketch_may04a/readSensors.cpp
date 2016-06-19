#include "readSensors.h"

// Read Values at ADCs and converts into Volt.
float readVccSense(){
  return analogRead(VCC_ADC) * (SUPPLY_VOLTAGE / ADC_MAX_VALUE);
}
float readGndSense(){
  return analogRead(GND_ADC)*(SUPPLY_VOLTAGE / ADC_MAX_VALUE);
}
float readSwitchSense1(){
  return analogRead(SWITCH_1_ADC) * (SUPPLY_VOLTAGE / ADC_MAX_VALUE);
}
float readSwitchSense2(){
  return analogRead(SWITCH_2_ADC) * (SUPPLY_VOLTAGE / ADC_MAX_VALUE);
}
int readIgnitionSense(){
  return digitalRead(IGNITION_PIN);
}
int readThrottleSense(){
  return digitalRead(THROTTLE_PIN);
}
