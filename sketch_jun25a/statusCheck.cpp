#include "statusCheck.h"
#include "Arduino.h"
#include <Wire.h>

static unsigned int aliveCounter = 0; //default

boolean checkSlaveResults(systemState masterSysState, boolean &isFirstRequest){
  boolean checkSuccessful = false;
  boolean connectionAndSlaveOk = false;
  boolean resultsConsistent = false;
  systemState slaveSysState = UNDEFINED_STATE;
  
  Wire.requestFrom(8, 2);      // request 6 bytes from slave device #8
  
  byte newAliveCounter;
  
  unsigned int messagePart = 0;
  while (Wire.available()) { // slave may send less than requested
    byte answerPart = Wire.read();
    if(messagePart == 0){
      newAliveCounter = answerPart;
    }
    if(messagePart == 1){
      slaveSysState = (systemState)answerPart;
    }
    messagePart++;
  }
    
  if(isFirstRequest == true){
    connectionAndSlaveOk = true;
    aliveCounter = newAliveCounter;
    if(aliveCounter >= 255){
    aliveCounter = 20; //reset
  }
  isFirstRequest = false;
  }
    
  if(newAliveCounter == (aliveCounter + 1)){
    connectionAndSlaveOk = true;
    aliveCounter = newAliveCounter;
    if(aliveCounter >= 255){
      aliveCounter = 20; //reset
    }
  }
  if(masterSysState == slaveSysState){
    resultsConsistent = true;
  }
  
  checkSuccessful = true;//(connectionAndSlaveOk && resultsConsistent);
  
  Serial.println("Master:     Slave:   ");
  Serial.print(masterSysState);
  Serial.print("               ");
  Serial.print(slaveSysState);
  Serial.println();
  Serial.print(aliveCounter);
  Serial.print("               ");
  Serial.print(newAliveCounter);
  Serial.println();
  
  Serial.println("Successful?: ");
  Serial.println(checkSuccessful);

    return checkSuccessful;
}
