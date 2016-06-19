/* Version: 04-05-16
   
   There are still some Error-Messages because of unexpected measurement-values,
   if switch is in the "unsynchronized" position.
*/

#include "Servo.h"
#include <Wire.h>
#include "brake.h"
#include "servoControl.h"
#include "constants.h"
#include "evaluation.h"
#include "readSensors.h"

/*********************************************************  
**********************************************************
**    Variable declarations initialization / resets     **
**********************************************************
*********************************************************/

Servo brakeSimulator;
byte brakeStatus;
int aliveCounter = 0; //TODO: start with higher value, e.g. 20
boolean ignitionOn = false;
boolean noError = true;
int ignitionSense = 1; //ignition off


/*********************************************************
**********************************************************
**                 Method declarations                  **
**********************************************************
*********************************************************/

void changeToSafeState(){
  digitalWrite(ERROR_LED, HIGH);
  noError = false;
}

void waitForIgnitionOn(){
  
  Serial.println("Ignition OFF!");
  
  //Turn LEDs off, when ignition is turned off
  digitalWrite(INFO_LED, LOW);
  digitalWrite(ERROR_LED, LOW);
  
  while(!ignitionOn){
    ignitionSense = readIgnitionSense();
    ignitionOn = evaluateIgnitionSense(ignitionSense);
    delay(50);
  }
  
  Serial.println("Ignition ON!");
  
  // Checks if brake-initialization was successful
  if (brakeInit() != true) {
    Serial.println("Can't initialise brake - stopping.");
    while(1);
  }
  else{
    // Initialization successful!
  }
}

// Supporting function for printing simple error-messages.
void reportError(String errorType){
  Serial.println(errorType);
}

// Supporting function for printing messages, if switch is not in neutral position.
void reportSwitchPress(String pressedType){
  Serial.println(pressedType);
}

boolean checkSlaveResults(systemState sysStateFromMaster){
  systemState sysStateFromSlave = UNDEFINED_STATE;
  byte fromSlave[2];
  int j = 0;
  Wire.requestFrom(8, 2);    // request 6 bytes from slave device #8
  while (Wire.available() && j < 2) { // slave may send less than requested
    byte c = Wire.read();
    fromSlave[j] = c;
    j++;
  }
  sysStateFromSlave = (systemState) fromSlave[1];
  int aliveCounterFromSlave = (int) fromSlave[0];
  if(aliveCounterFromSlave > aliveCounter){
    aliveCounter = aliveCounterFromSlave;
    if(aliveCounter >= 255){
      aliveCounter = 0;
    }
  }
  else{
//    Serial.println("AliveCounter not valid!");
  }
//  Serial.println("AliveCounter:     Slave:            Master: ");
//  Serial.print(aliveCounterFromSlave);
//  Serial.print("                  ");
//  Serial.print(sysStateFromSlave);
//  Serial.print("                  ");
//  Serial.println(sysStateFromMaster);
//  Serial.println();
  
  return true; //solange das hier nicht passt wird defaultmaessig immer true zurückgegeben
}


/********************************************************* 
**********************************************************
**                      setup()                         **
**********************************************************
*********************************************************/

void setup() {
  
  brakeSimulator.attach(SERVO_PIN);           //Attach Servo to PIN 9~
  pinMode(LED, OUTPUT);                       // Initializing led-pin as an output.
  pinMode(ERROR_LED, OUTPUT);                 // Initializing led-pin as an output.
  pinMode(INFO_LED, OUTPUT);                  // Initializing led-pin as an output.
  pinMode(IGNITION_PIN, INPUT);
  pinMode(THROTTLE_PIN, INPUT);
  analogReadResolution(ADC_READ_RESOLUTION);  // Sets the size (in bits) of the value returned by analogRead(). 
  Serial.begin(115200);                       // Sets the data rate for communication with the computer - 115200 is one of the default rates.       
  
  waitForIgnitionOn();
  
  Wire.begin();        // join i2c bus (address optional for master)
}

/********************************************************* 
**********************************************************
**                       loop()                         **
**********************************************************
*********************************************************/

void loop() {

  int count = 0;
  int oldCount = 0;
  //TODO: check which value is appropriate as default for startTime
  unsigned long startTime;
  unsigned long passedTime = 0;
  
  while(ignitionOn && noError){
    
    if(oldCount < count){
      oldCount = count;
    } else {
      count = 0;
      oldCount = 0;
      passedTime = 0;
    }
    
    Serial.println(count);
    Serial.println(passedTime);
    
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
    int servoPos = START_POS;                         // servoPos - position of servo - [20,160]
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
    
    
    ignitionSense = readIgnitionSense(); //for stopping while loop
    ignitionOn = evaluateIgnitionSense(ignitionSense); //to stopping while-loop
    
    if(!vccStatusOk){
      reportError("Error: VCC");
      noError = false;  
    }
    if(!gndStatusOk){
      reportError("Error: GND");
      noError = false;
    }
    
    if(vccStatusOk && gndStatusOk){
      // Reasons Brake-/Switch/Circuit-State based on measured ADC-values and defined voltage intervals.
      sysState = analyseSystemState(switchStatus1, switchStatus2, switch1, switch2);
    }

    boolean slaveResultsOk = checkSlaveResults(sysState);
    
    if(!slaveResultsOk){
      reportError("Error: Slave-Results!");
      noError = false;
    }
    
    
    
    //applySystemState(sysState)      //TODO : find better name for function!
    //!!!Achtung gndOK und vccOK noch mit berücksichtigen
    switch (sysState){
      case NOTHING_PRESSED:
        if(throttlePressed){
        releaseBrake();
        }
        break;
      case APPLY_PRESSED:
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
             //startTime = 0;
             count = 0;
             oldCount = 0;
          }
          else{        
            count++;
          }        
        }
        break;
      case RELEASE_PRESSED:
        releaseBrake();
        break;
      case SHORT_VCC_C:
        reportError("Short VCC -> C.");
        noError = false;
        break;
      case SHORT_VCC_E:
        reportError("Short VCC -> E.");
        noError = false;
        break;
      case SHORT_GND_D:
        reportError("Short GND -> D.");
        noError = false;
        break;
      case SHORT_GND_F:
        reportError("Short GND -> F.");
        noError = false;
        break;
      case OPEN_RES_G:
        reportError("Open res @ G.");
        noError = false;
        break;
      case OPEN_RES_H:
        reportError("Open res @ H.");
        noError = false;
        break;
      default:
        reportError("Unknown System State!");
        noError = false;
      break;   
    }
    //control servo according to brake-state
    brakeStatus = getBrakeStatus();
    servoPos = getServoPos(brakeStatus);
    if((servoPos >= START_POS) && (servoPos <= END_POS)){    // check value, before passing it to a library function! MISRA-Dir. 4.11
      brakeSimulator.write(servoPos);
    }  
  } // end of while(ignitionOn && noError) 
  
  //need to check ignition-state,  because we are not in the while-loop anymore
  ignitionSense = readIgnitionSense();
  ignitionOn = evaluateIgnitionSense(ignitionSense);
   
  if(!noError){
    changeToSafeState();
  }
   
  if(!ignitionOn){
    waitForIgnitionOn();
  }
 
 } // end of loop()
