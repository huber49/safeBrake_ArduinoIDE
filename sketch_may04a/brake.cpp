/* Has to be .cpp and not .c
*  reason: see notes in "Notizen zur Programmentwicklung.doc"
*  Version: 04-05-16
*/
#include "brake.h"
#include "constants.h"

static byte brakeState = 0;

byte getBrakeStatus() {
  return brakeState;
}

void applyBrake() {
  if(brakeState != BRAKE_APPLIED){
    brakeState = BRAKE_APPLIED;
    // control servo
    Serial.println("Stop Servo.");
    digitalWrite(INFO_LED, HIGH);
  }
}

void releaseBrake() {
  if(brakeState != BRAKE_RELEASED){
    brakeState = BRAKE_RELEASED;
    //  control servo
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
