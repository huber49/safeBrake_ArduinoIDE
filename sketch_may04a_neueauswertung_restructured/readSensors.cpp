#include "readSensors.h"

unsigned int readVccSense(){
  return analogRead(VCC_ADC);
}
unsigned int readGndSense(){
  return analogRead(GND_ADC);
}
unsigned int readSwitchSense1(){
  return analogRead(SWITCH_1_ADC);
}
unsigned int readSwitchSense2(){
  return analogRead(SWITCH_2_ADC);
}

unsigned int readIgnitionSense(){
  return digitalRead(IGNITION_PIN);
}
unsigned int readThrottleSense(){
  return digitalRead(THROTTLE_PIN);
}

// Read Values at ADCs and converts into Volt.
void getSensorValues(unsigned int sensorValues[]){
    unsigned int vcc = 0;
    unsigned int gnd = 0;
    unsigned int sense1 = 0;
    unsigned int sense2 = 0;
    unsigned int ignition = 0;
    unsigned int throttle = 0;
    
    unsigned int vccTmp = 0;
    unsigned int gndTmp = 0;
    unsigned int sense1Tmp = 0;
    unsigned int sense2Tmp = 0;
    unsigned int ignitionTmp = 0;
    unsigned int throttleTmp = 0;
    
    for(unsigned int i = 0; i < 16; i++){
      vccTmp += readVccSense();
      gndTmp += readGndSense();
      sense1Tmp += readSwitchSense1();
      sense2Tmp += readSwitchSense2();
      ignitionTmp += readIgnitionSense();
      throttleTmp += readThrottleSense();
    }
    
    vcc = vccTmp / 16;
    gnd = gndTmp / 16;
    sense1 = sense1Tmp / 16;
    sense2 = sense2Tmp / 16;
    ignition = ignitionTmp / 16;
    throttle = throttleTmp / 16;
    
    sensorValues[0] = vcc;
    sensorValues[1] = gnd;
    sensorValues[2] = sense1;
    sensorValues[3] = sense2;
    sensorValues[4] = ignition;
    sensorValues[5] = throttle;
}
