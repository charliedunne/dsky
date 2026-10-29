#include "EnableScreenSaver.h"

// Configuration
#include "ListOfActions.h"

// Logging
#include "Logger.h"

EnableScreenSaver::EnableScreenSaver(VariableStore &store, HmiData &data) :
  Action(ENABLE_SCREENSAVER_VERB, ENABLE_SCREENSAVER_NOUN, data, ACTION_SINGLE, "SCREENSAVER ON"),
  store_(store)
{
}

void EnableScreenSaver::operation()
{
  LogTrace << "Running ScreenSaver ON" << std::endl;

  /* set() persists the value and triggers the apply callback that
   * enables the screensaver. Do NOT throw on failure: an exception
   * escaping operation() is uncaught in the action thread and would
   * terminate the process, so log the error instead. */
  if (!store_.set(VAR_NOUN_SCREENSAVER, 1))
  {
    LogError << "Failed to enable screensaver" << std::endl;
  }
}
