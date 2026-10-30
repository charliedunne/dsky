#include "WriteVar.h"

// Configuration
#include "HmiData.h"
#include "ListOfActions.h"

// Logging
#include "Logger.h"

WriteVar::WriteVar(VariableStore &store, int noun, HmiData &data) :
  Action(WRITE_VAR_VERB, noun, data, ACTION_SINGLE, "WRITE VAR"),
  store_(store)
{
}

void WriteVar::operation()
{
  const int value = hmiData_.getRightLcdData().nR2;

  LogTrace << "Writing variable NOUN " << getNoun() << " value " << value << std::endl;

  /* set() validates the value (bools strictly 0/1) and persists it.
   * Do NOT throw on failure: an exception escaping operation() is
   * uncaught in the action thread and would terminate the process,
   * so log the error instead. */
  if (!store_.set(getNoun(), value))
  {
    LogError << "Failed to write variable NOUN " << getNoun() << " value " << value << std::endl;
  }
}
