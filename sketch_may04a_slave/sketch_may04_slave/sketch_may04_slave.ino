/* Version: 04-05-16
   
   There are still some Error-Messages because of unexpected measurement-values,
   if switch is in the "unsynchronized" position.
*/

#include <Wire.h>
#include "constants.h"
#include "evaluation.h"
#include "readSensors.h"

/*********************************************************  
**********************************************************
**    Variable declarations initialization / resets     **
**********************************************************
*********************************************************/

int aliveCounter = 20;
systemState sysState = UNDEFINED_STATE;
boolean receivedRequest = false;

/*********************************************************
**********************************************************
**                 Method declarations                  **
**********************************************************
*********************************************************/

// Supporting function for printing simple error-messages.
void reportError(String errorType){
  Serial.println(errorType);
  //digitalWrite(ERROR_LED, HIGH);
}

// Supporting function for printing messages, if switch is not in neutral position.
void reportSwitchPress(String pressedType){
  Serial.println(pressedType);
}

void requestEvent() {
  aliveCounter++;
  if(aliveCounter >= 256){
    aliveCounter = 20; // reset -> max. 1 byte
  }
  byte slaveSysState = (byte) sysState;
  byte answer[] = {aliveCounter, slaveSysState};
  Wire.write(answer, 2);
}

/********************************************************* 
**********************************************************
**                      setup()                         **
**********************************************************
*********************************************************/

void setup() {
 
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event

  pinMode(LED, OUTPUT);                       // Initializing led-pin as an output.
  
  pinMode(IGNITION_PIN, INPUT);
  pinMode(THROTTLE_PIN, INPUT);
  
  analogReadResolution(ADC_READ_RESOLUTION);  // Sets the size (in bits) of the value returned by analogRead(). 
  
  Serial.begin(115200);                       // Sets the data rate for communication with the computer - 115200 is one of the default rates.       
}

/********************************************************* 
**********************************************************
**                       loop()                         **
**********************************************************
*********************************************************/

void loop() {
  
  if(receivedRequest){
    Serial.println("Received Request");
    receivedRequest = false;
  }
  
  /********************************************************* 
  **    Variable declarations initialization / resets     **
  *********************************************************/
 
  float vccSense = 0;                               // vccSense - measured VCC-control voltage 
  float gndSense = 0;                               // gndSense - measured GND-control voltage
  float switch1 = 0;                                // switch1 - measured voltage of first switch-part
  float switch2 = 0;                                // switch2 - measured voltage of second switch-part
  int throttleSense = 1;                            // 1 = no throttle
  boolean vccStatusOk = 0;                          // vccStatusOk - true, if there is no short or opening in the signal circuit
  boolean gndStatusOk = 0;                          // gndStatusOk - true, if there is no short or opening in the signal circuit
  voltInterval switchStatus1 = UNDIFINED_INTERVAL;  // switchStatus1 - Indicates, in which range the first measured switch-value was.
  voltInterval switchStatus2 = UNDIFINED_INTERVAL;  // switchStatus2 - Indicates in which range the second measured switch-value was.
  boolean throttlePressed = false;
  systemState sysState = UNDEFINED_STATE;           // sysState - code for 'switch-position' or hardware-failure-type             
  
  
  
  
  /********************************************************* 
  **              Methods / "Working Part"                **
  *********************************************************/
  
  // Switch led on and off, to signalize that the programme is running.
  digitalWrite(LED, HIGH);
  delay(50);
  digitalWrite(LED, LOW);
  
  vccSense = readVccSense();
  gndSense = readGndSense();
  switch1 = readSwitchSense1();
  switch2 = readSwitchSense2();
  throttleSense = readThrottleSense();
    
  vccStatusOk = evaluateVccSense(vccSense);
  gndStatusOk = evaluateGndSense(gndSense);
  switchStatus1 = evaluateSwitch(switch1);
  switchStatus2 = evaluateSwitch(switch2);
  throttlePressed = evaluateThrottleSense(throttleSense);

  if(!vccStatusOk){reportError("Error: VCC");}
  if(!gndStatusOk){reportError("Error: GND");}
  
  if(vccStatusOk && gndStatusOk){
    // Reasons Brake-/Switch/Circuit-State based on measured ADC-values and defined voltage intervals.
    sysState = analyseSystemState(switchStatus1, switchStatus2, switch1, switch2);
  }
  
  //applySystemState(sysState)      //TODO : find better name for function!
  //!!!Achtung gndOK und vccOK noch mit berücksichtigen
  switch (sysState){
    case NOTHING_PRESSED:
        if(throttlePressed){
        Serial.println("Throttle pressed.");
        }
        break;
    case APPLY_PRESSED:
      Serial.println("Apply pressed.");
      break;
    case RELEASE_PRESSED:
      Serial.println("Release pressed.");
      break;
    case SHORT_VCC_C:
      reportError("Short VCC -> C.");
      break;
    case SHORT_VCC_E:
      reportError("Short VCC -> E.");
      break;
    case SHORT_GND_D:
      reportError("Short GND -> D.");
      break;
    case SHORT_GND_F:
      reportError("Short GND -> F.");
      break;
    case OPEN_RES_G:
      reportError("Open res @ G.");
      break;
    case OPEN_RES_H:
      reportError("Open res @ H.");
      break;
    case UNSYNC_SWITCH_BEHAVE:
        //ignore
        break;
     default:
        reportError("Unknown System State!");
     break;   
  }
        
 }
