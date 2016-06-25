#include "failureHandler.h"

// Supporting function for printing simple error-messages.
void reportError(String errorType){
  Serial.println(errorType);
}

void changeToSafeState(systemState sysState, boolean &noError){
  digitalWrite(ERROR_LED, HIGH);
  noError = false;
  switch(sysState){
    case SHORT_VCC_C:
      Serial.println("HW-Failure: Short @ VCC->C");
      break;    
    case SHORT_VCC_E:
      Serial.println("HW-Failure: Short @ VCC->E");
      break;
    case SHORT_GND_D:
      Serial.println("HW-Failure: Short @ GND->D");
      break;
    case SHORT_GND_F:
      Serial.println("HW-Failure: Short @ GND->F");
      break;
    case OPEN_RES_G:
      Serial.println("HW-Failure: Open @ G");
      break;
    case OPEN_RES_H:
      Serial.println("HW-Failure: Open @ H");
      break;
    case UNDEFINED_STATE:
      Serial.println("Failure: UNDEFINED_STATE");
    default
      Serial.println("Failure: Unspecified Failure");
  }
}
