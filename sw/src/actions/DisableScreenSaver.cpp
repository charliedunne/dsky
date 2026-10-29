#include "DisableScreenSaver.h"

// Configuration
#include "ListOfActions.h"

// Logging
#include "Logger.h"

DisableScreenSaver::DisableScreenSaver(VariableStore &store, HmiData &data) :
  Action(DISABLE_SCREENSAVER_VERB, DISABLE_SCREENSAVER_NOUN, data, ACTION_SINGLE, "SCREENSAVER OFF"),
  store_(store)
{
}

void DisableScreenSaver::operation()
{
  LogTrace << "Running ScreenSaver OFF" << std::endl;

  /* set() persists the value and triggers the apply callback that
   * disables the screensaver. Do NOT throw on failure: an exception
   * escaping operation() is uncaught in the action thread and would
   * terminate the process, so log the error instead. */
  if (!store_.set(VAR_NOUN_SCREENSAVER, 0))
  {
    LogError << "Failed to disable screensaver" << std::endl;
  }
}
