#include "ReadVar.h"

// Configuration
#include "HmiData.h"
#include "ListOfActions.h"

// Logging
#include "Logger.h"

ReadVar::ReadVar(VariableStore &store, int noun, HmiData &data) :
  Action(READ_VAR_VERB, noun, data, ACTION_PERIODIC, "READ VAR"),
  store_(store)
{
  setPeriod(1000);
}

void ReadVar::operation()
{
  int value = UNASIGNED;

  if (store_.get(getNoun(), value))
  {
    hmiData_.rSetStrR1("d", DRAW_BLINK);
    hmiData_.rSetnR2(value, DRAW_ON);
    hmiData_.rSetR2Label(DRAW_ON);
  }
}
