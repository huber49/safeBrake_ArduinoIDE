#ifndef FailureHandler_h
#define FailureHandler_h

#include "Arduino.h"
#include "constants.h"

void reportError(String errorType);

void changeToSafeState(systemState sysState, boolean &noError);

#endif
