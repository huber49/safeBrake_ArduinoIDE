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
**  Variable declarations declaration / initialization  **
**********************************************************
*********************************************************/

Servo brakeSimulator;                 // the servo represents the state of the brake
byte brakeStatus;                     // Applied / Released

boolean ignitionOn = false;           
boolean noError = true;               // if system is ok / no error detected
byte aliveCounter = 20;               // alivecounter for statusCheck with slave
boolean isFirstRequest = true;          // for synchronizing alivecounter at the first statusCheck with slave

/*********************************************************
**********************************************************
**                 Method declarations                  **
**********************************************************
*********************************************************/

void waitForIgnitionOn(){
  unsigned int ignitionSense = 1;
  
  Serial.println("Ignition OFF!");
  digitalWrite(INFO_LED, LOW);       //Turn LEDs off, when ignition is turned off
  digitalWrite(ERROR_LED, LOW);      //Turn LEDs off, when ignition is turned off
  
  while(ignitionOn == false){
    ignitionSense = readIgnitionSense();
    ignitionOn = evaluateIgnitionSense(ignitionSense);
    delay(50);
  }
  
  Serial.println("Ignition ON!");
  
  if (brakeInit() != true) {
    Serial.println("Can't initialise brake - stopping.");
    digitalWrite(ERROR_LED, HIGH);
    while(1);
  }
  else{
    // Initialization successful!
  }
}

// Supporting function for printing messages, if switch is not in neutral position.
void reportSwitchPress(String pressedType){
  Serial.println(pressedType);
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
  
  int count = 0;      // counter for applieng brake while throttle is pressed
  int oldCount = 0;   // counter for applieng brake while throttle is pressed
  //TODO: check which value is appropriate as default for startTime
  unsigned long startTime;
  unsigned long passedTime = 0;
  
  while(ignitionOn && noError){
    
    // for timeMeasurement, if someone wants to apply brake while throttle is pressed
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
   
    float switch1 = 0;                                // switch1 - measured voltage of first switch-part
    float switch2 = 0;                                // switch2 - measured voltage of second switch-part
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
    delay(500);
    digitalWrite(LED, LOW);
    
    unsigned int sensorValues[6];
    unsigned int *pointer; //TODO Überprüfe den Umgang mit dem Array
    pointer = sensorValues;
    getSensorValues(pointer);
    
    Serial.println("Sensorvalues: ");
    unsigned int testint = sensorValues[2];
    Serial.println(testint);
        
    vccStatusOk = evaluateVccSense(sensorValues[0]);
    gndStatusOk = evaluateGndSense(sensorValues[1]);
    switchStatus1 = evaluateSwitch(sensorValues[2]);
    switchStatus2 = evaluateSwitch(sensorValues[3]);
    throttlePressed = evaluateThrottleSense(sensorValues[5]);
    ignitionOn = evaluateIgnitionSense(sensorValues[4]); //to stopping while-loop
    
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
      case UNSYNC_SWITCH_BEHAVIOUR:
        //ignore
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
   ignitionOn = evaluateIgnitionSense(sensorValues[4]);
   
   if(!noError){
    changeToSafeState();
   }
   if(!ignitionOn){
    waitForIgnitionOn();
   }
 
 } // end of loop()
