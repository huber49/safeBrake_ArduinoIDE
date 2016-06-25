/* Has to be .cpp and not .c
*  reason: see notes in "Notizen zur Programmentwicklung.doc"
*  Version: 04-05-16
*/
#include "brake.h"
#include "constants.h"

static byte brakeState = 0;

static unsigned int servoPos = START_POS;
byte simulatorDirection = TURN_RIGHT;

byte getBrakeStatus() {
  return brakeState;
}

void applyBrake() {
  if(brakeState != BRAKE_APPLIED){
    brakeState = BRAKE_APPLIED;
    Serial.println("Stop Servo.");
    digitalWrite(INFO_LED, HIGH);
  }
}

void releaseBrake() {
  if(brakeState != BRAKE_RELEASED){
    brakeState = BRAKE_RELEASED;
    Serial.println("Start Servo.");
    digitalWrite(INFO_LED, LOW);
  }
}

boolean brakeInit() {
  /* Set up servo to "Brake-Applied" position */
  Serial.println("Brake-Initialization");
  // for showing, that led are working
  digitalWrite(INFO_LED, HIGH);
  digitalWrite(ERROR_LED, HIGH);
  delay(1000);
  digitalWrite(INFO_LED, LOW);
  digitalWrite(ERROR_LED, LOW);
  if(brakeState == 0){
    applyBrake();
  }
  else{
    if(brakeState == BRAKE_RELEASED){
      //Info-LED is already low
    }
    else if(brakeState == BRAKE_APPLIED){
      digitalWrite(INFO_LED, HIGH);
    }
  }
  return true;
}

unsigned int getServoPos(){
  // change direction if servo is at it's limits
  if(servoPos >= END_POS){
    simulatorDirection = TURN_LEFT;
  }
  if(servoPos <= START_POS){
    simulatorDirection = TURN_RIGHT;
  }
  
  // change position, according to brakestate
  if(brakeState == BRAKE_RELEASED){
    if(simulatorDirection == TURN_RIGHT){
      servoPos += 4;
    }
    else if(simulatorDirection == TURN_LEFT){
      servoPos -= 4;
    }
  }
  else if(brakeState == BRAKE_APPLIED){
    servoPos = START_POS;
  }
  else{
    Serial.print("Error: BrakeState-Evaluation!");
  }
  return servoPos;
}

void adjustServo(Servo &servo){
   servoPos = getServoPos();
   if((servoPos >= START_POS) && (servoPos <= END_POS)){    // check value, before passing it to a library function! MISRA-Dir. 4.11
      servo.write(servoPos);
   } 
}
