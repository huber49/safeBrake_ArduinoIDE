#include "ignition.h"
#include "readSensors.h"
#include "evaluation.h"
#include "brake.h"

void waitForIgnitionOn(){
  unsigned int ignitionSense = 1;
  boolean ignitionOn = false;
  
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
