#ifndef ServoControl_h
#define ServoControl_h

#include "Arduino.h"
#include "brake.h"
#include "constants.h"

#define START_POS 20
#define END_POS 160
#define TURN_RIGHT 20
#define TURN_LEFT 21

/*
  Calculates the Position of the servo.
  return: new Servoposition
*/
int getServoPos(byte brakeState);

#endif
