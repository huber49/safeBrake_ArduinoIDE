/* 
*  Version: 04-05-16
*/
#ifndef Brake_h
#define Brake_h

#include "Arduino.h"
#include "Servo.h"

#define BRAKE_APPLIED 10
#define BRAKE_RELEASED 11
#define START_POS 20
#define END_POS 160
#define TURN_RIGHT 20
#define TURN_LEFT 21

/* brakeStatus - BRAKE_APPLIED 10, BRAKE_RELEASED 11
*/
byte getBrakeStatus();

/* Applies the brake.
*/  
void applyBrake();

/* Releases the brake.
*/
void releaseBrake();

/* Initialises the brake.
*/
boolean brakeInit();

void adjustServo(Servo &servo);

#endif

