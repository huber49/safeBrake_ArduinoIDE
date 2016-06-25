/* Version: 04-05-16
   
   There are still some Error-Messages because of unexpected measurement-values,
   if switch is in the "unsynchronized" position.
*/

#include "Servo.h"
#include <Wire.h>
#include "brake.h"
#include "constants.h"
#include "evaluation.h"
#include "readSensors.h"
#include "ignition.h"
#include "failureHandler.h"
#include "statusCheck.h"

/*********************************************************
**  Variable declarations declaration / initialization  **
*********************************************************/

Servo brakeSimulator;                 // the servo represents the state of the brake
byte brakeStatus;                     // Applied / Released

boolean ignitionOn;           
boolean noError;               // if system is ok / no error detected
boolean isFirstRequest;          // for synchronizing alivecounter at the first statusCheck with slave

/*********************************************************
**                      setup()                         **
*********************************************************/

void setup() {
  
  // HW specific
  brakeSimulator.attach(SERVO_PIN);           //Attach Servo to PIN 9~
  pinMode(LED, OUTPUT);                       // Initializing led-pin as an output.
  pinMode(ERROR_LED, OUTPUT);                 // Initializing led-pin as an output.
  pinMode(INFO_LED, OUTPUT);                  // Initializing led-pin as an output.
  pinMode(IGNITION_PIN, INPUT);
  pinMode(THROTTLE_PIN, INPUT);
  analogReadResolution(ADC_READ_RESOLUTION);  // Sets the size (in bits) of the value returned by analogRead(). 
  Serial.begin(115200);                       // Sets the data rate for communication with the computer - 115200 is one of the default rates.       
  Wire.begin();                               // join i2c bus
  
  // not HW specific
  ignitionOn = false;
  noError = true;
  isFirstRequest = true;
  
  waitForIgnitionOn();
}

/*********************************************************
**                       loop()                         **
*********************************************************/

void loop() {
  
  int count = 0;      // counter for applieng brake while throttle is pressed
  int oldCount = 0;   // counter for applieng brake while throttle is pressed
  unsigned long startTime;   //TODO: check which value is appropriate as default for startTime
  unsigned long passedTime = 0;
  
  unsigned int switch1;                                // switch1 - measured voltage of first switch-part
  unsigned int switch2;                                // switch2 - measured voltage of second switch-part
  boolean vccStatusOk;                          // vccStatusOk - true, if there is no short or opening in the signal circuit
  boolean gndStatusOk;                          // gndStatusOk - true, if there is no short or opening in the signal circuit
  boolean throttlePressed;
  voltInterval switchStatus1;  // switchStatus1 - Indicates, in which range the first measured switch-value was.
  voltInterval switchStatus2;  // switchStatus2 - Indicates in which range the second measured switch-value was.
  
  byte brakeState;
  
  unsigned int servoPos;       // servoPos - position of servo - [20,160]
  systemState sysState;        // sysState - code for 'switch-position' or hardware-failure-type
  
  while((ignitionOn == true)&& (noError == true)){
    
    // Switch led on and off, to signalize that the programme is running.
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
  
    /********************************************************* 
    **                    Variable resets                   **
    *********************************************************/
         
    vccStatusOk = false;                          
    gndStatusOk = false;                          
    throttlePressed = false;
    switchStatus1 = UNDIFINED_INTERVAL;  
    switchStatus2 = UNDIFINED_INTERVAL;  
    servoPos = START_POS;                
    sysState = UNDEFINED_STATE;          

    // for timeMeasurement, if someone wants to apply brake while throttle is pressed
    if(oldCount < count){
      oldCount = count;
    } else {
      count = 0;
      oldCount = 0;
      passedTime = 0;
    }
    
    /********************************************************* 
    **              Methods / "Working Part"                **
    *********************************************************/
    
    unsigned int sensorValues[6];
    unsigned int *pointer; //TODO Überprüfe den Umgang mit dem Array
    pointer = sensorValues;
    getSensorValues(pointer);
        
    vccStatusOk = evaluateVccSense(sensorValues[0]);
    gndStatusOk = evaluateGndSense(sensorValues[1]);
    switchStatus1 = evaluateSwitch(sensorValues[2]);
    switchStatus2 = evaluateSwitch(sensorValues[3]);
    ignitionOn = evaluateIgnitionSense(sensorValues[4]);
    throttlePressed = evaluateThrottleSense(sensorValues[5]);
    
    if(vccStatusOk == false){
      sysState = VCC_ERROR;
      changeToSafeState(VCC_ERROR, noError);
    }
    if(!gndStatusOk == false){
      sysState = GND_ERROR; //could overwrite VCC_ERROR
      changeToSafeState(GND_ERROR, noError);
    }
    
    if((vccStatusOk == true) && (gndStatusOk == true)){
      // Reasons Brake-/Switch/Circuit-State based on measured ADC-values and defined voltage intervals.
      sysState = analyseSystemState(switchStatus1, switchStatus2/*, switch1, switch2*/);
    }

    boolean slaveResultsOk = checkSlaveResults(sysState, isFirstRequest);
//    if(isFirstRequest == true){
//      isFirstRequest = false;
//    } //sollte in ignition.cpp gesetzt werden
    
    if(slaveResultsOk == true){
      reportError("Error: Slave-Results!");
      noError = false;
    }
    
    
    if (sysState == VCC_ERROR){
      //job already done
    }
    else if (sysState == GND_ERROR){
      //job already done
    }    
    else if (sysState == NOTHING_PRESSED){
      if(throttlePressed){
      releaseBrake();
      }
    } 
    else if (sysState == APPLY_PRESSED){
      if(!throttlePressed){
        applyBrake();
      }
      else {
        Serial.println("Tried to apply Brake during Throttle was pressed!");
        
        if(count == 0){
          startTime = millis();
        }
        else if(count > 0){
          passedTime = millis() - startTime;  
        }
        
        if(passedTime >= 2500){
          applyBrake();
          passedTime = 0;
          count = 0;
          oldCount = 0;
        }
        else{        
          count++;
        }        
      }
    }
    else if (sysState == RELEASE_PRESSED){
      releaseBrake();
    }
    else if ( (sysState == SHORT_VCC_C) || (sysState == SHORT_VCC_E) || 
              (sysState == SHORT_GND_D) || (sysState == SHORT_GND_F) ||
              (sysState == OPEN_RES_G) || (sysState == OPEN_RES_H) ){
      changeToSafeState(sysState, noError);
    }
    else if (sysState == UNSYNC_SWITCH_BEHAVIOUR){
        //ignore
    }
    else{
      changeToSafeState(sysState, noError); //systate-default: UNDEFINED_STATE
    }   
    
    adjustServo(brakeSimulator);
  } // end of while(ignitionOn && noError) 
  
  unsigned int ignitionSense = readIgnitionSense(); //need to check ignition-state,  because we are not in the while-loop anymore
  ignitionOn = evaluateIgnitionSense(ignitionSense);
  
  if(noError == false){
    changeToSafeState(UNDEFINED_STATE, noError);
  }
  if(ignitionOn == false){ 
    waitForIgnitionOn();
  }
 
 } // end of loop()
