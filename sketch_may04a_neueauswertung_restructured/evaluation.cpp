#include "evaluation.h"

boolean evaluateVccSense(unsigned int measuredVcc){
  boolean vccOk = false;
  if(measuredVcc >= VCC_GOOD){
    vccOk = true;
  }
  return vccOk;
}

boolean evaluateGndSense(unsigned int gndVoltage){
  boolean gndOk = false;
  if(gndVoltage <= GND_GOOD){
    gndOk = true;
  }
  return gndOk;
}
boolean evaluateIgnitionSense(unsigned int ignitionSense){
  boolean ignitionOn = false;
  if(ignitionSense == 0){
    ignitionOn = true;
  }
  return ignitionOn;
}
boolean evaluateThrottleSense(unsigned int throttleSense){
  boolean throttleOn = false;
  if(throttleSense == 0){
    throttleOn = true;
  }
  return throttleOn;
}
voltInterval evaluateSwitch(unsigned int voltage){
  voltInterval returnStatus = UNDIFINED_INTERVAL; //check, if NULL is ok as default - what happens if NULL is returned?
  if((voltage >= 0) && (voltage <= 125)){
    returnStatus = FAIL_GND_SHORT;
  } // 0 to 0,1V 
  else if((voltage >= 370) && (voltage <= 625)){
    returnStatus = SWITCH_LOW_VAL;
  } // 0,3 to 0,5V 
  else if((voltage >= 805) && (voltage <= 935)){
    returnStatus = FAIL_IGNORE;
  } // 0,65 to 0,75V 
  else if((voltage >= 990) && (voltage <= 1120)){
    returnStatus = FAIL_IGNORE;
  } // 0,80 to 0,90V 
  else if((voltage >= 1240) && (voltage <= 1490)){
    returnStatus = SWITCH_NORM_VAL;
  } // 1,00 to 1,20
  else if((voltage >= 1920) && (voltage <= 2110)){
    returnStatus = FAIL_OPEN;
  } // 1,55 to 1,70
  else if((voltage >= 2295) && (voltage <= 2420)){
    returnStatus = FAIL_IGNORE;
  } // 1,85 to 1,95
  else if((voltage >= 2730) && (voltage <= 2855)){
    returnStatus = FAIL_IGNORE;
  } // 2,20 to 2,30
  else if((voltage >= 2975) && (voltage <= 3040)){
    returnStatus = FAIL_IGNORE;
  } // 2,40 to 2,45
  else if((voltage >= 3050) && (voltage <= 3230)){
    returnStatus = SWITCH_HIGH_VAL;
  } // 2,46 to 2,60 
  else if((voltage >= 3475) && (voltage <= 3540)){
    returnStatus = FAIL_IGNORE;
  } // 2,80 to 2,85
  else if((voltage >= 3720) && (voltage <= 4095)){
    returnStatus = FAIL_VCC_SHORT;
  } // 3,00 to 3,30
  else {
    Serial.print("evaluateError!");
    Serial.print("   ");
    Serial.println(voltage);    
  }
 return returnStatus;
}

systemState analyseSystemState(voltInterval switchStatus1, voltInterval switchStatus2/*, unsigned int switch1, unsigned int switch2*/){
  
  systemState sysState = UNDEFINED_STATE;  //TODO: comment
  
  if((switchStatus1 == SWITCH_NORM_VAL) && (switchStatus2 == SWITCH_NORM_VAL)){
    sysState = NOTHING_PRESSED;
  }
  else if((switchStatus1 == SWITCH_LOW_VAL) && (switchStatus2 == SWITCH_HIGH_VAL)){
    sysState = APPLY_PRESSED;    
  }
  else if((switchStatus1 == SWITCH_HIGH_VAL) && (switchStatus2 == SWITCH_LOW_VAL)){
    sysState = RELEASE_PRESSED;
  }
  //HW-Failures
  else if(switchStatus1 == FAIL_VCC_SHORT)  {sysState = SHORT_VCC_C;}
  else if(switchStatus2 == FAIL_VCC_SHORT)  {sysState = SHORT_VCC_E;}
  else if(switchStatus1 == FAIL_GND_SHORT)  {sysState = SHORT_GND_D;}
  else if(switchStatus2 == FAIL_GND_SHORT)  {sysState = SHORT_GND_F;}
  else if(switchStatus1 == FAIL_OPEN)       {sysState = OPEN_RES_G;}
  else if(switchStatus2 == FAIL_OPEN)       {sysState = OPEN_RES_H;}
  else if( ((switchStatus1 == SWITCH_LOW_VAL) && (switchStatus2 == SWITCH_NORM_VAL)) ||
           ((switchStatus1 == SWITCH_NORM_VAL) && (switchStatus2 == SWITCH_LOW_VAL)) ||
           ((switchStatus1 == SWITCH_HIGH_VAL) && (switchStatus2 == SWITCH_NORM_VAL)) ||
           ((switchStatus1 == SWITCH_NORM_VAL) && (switchStatus2 == SWITCH_HIGH_VAL)) ){
             sysState = UNSYNC_SWITCH_BEHAVIOUR;
    Serial.println("Detected unsynchronized-behaviour of the Switch Button --> Ignored!");
  }
  else{
      // Unknown combination of voltage-intervals - or lack of voltage interval(s)
      Serial.println("Unknown Error");
//      Serial.print("   ");
//      Serial.print(switch1);
//      Serial.print("   ");
//      Serial.println(switch2);
  }
  return sysState;
  
}
