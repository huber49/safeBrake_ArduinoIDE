#include "failureHandler.h"

// Supporting function for printing simple error-messages.
void reportError(String errorType){
  Serial.println(errorType);
}

void changeToSafeState(){
  digitalWrite(ERROR_LED, HIGH);
  noError = false;
}
