#ifndef Evaluation_h
#define Evaluation_h

#include "Arduino.h"
#include "constants.h"


boolean evaluateVccSense(unsigned int measuredVcc) __attribute__ ((noinline));

/* Checks, if gnd-voltage is ok.
   true - if there are no HW-failures in circuit.
*/
boolean evaluateGndSense(unsigned int gndVoltage) __attribute__ ((noinline));

boolean evaluateIgnitionSense(unsigned int ignitionSense) __attribute__ ((noinline));

boolean evaluateThrottleSense(unsigned int throttleSense) __attribute__ ((noinline));


/*
  only the return-status 1, 7 and 13 (and maybe 2) are relevant for the detection and identification of hardware-failures
  the other statuses are good to know if you want to double-check a certain hw-failure-detection, but they
  are not used in this version of the algorithm (maybe i can use the on the second board in an other algorithm)
  
  return-status 2, 5 and 11 are rekevant for the evaluation of the pressed-status of the switch-button.

  Note/TODO: Maybe it is better / nicer to use a map in this function, to map a voltage to an interval, that can be mapped to a status!
*/
voltInterval evaluateSwitch(unsigned int voltage);

systemState analyseSystemState(voltInterval switchStatus1, voltInterval switchStatus2, unsigned int switch1, unsigned int switch2);

#endif
