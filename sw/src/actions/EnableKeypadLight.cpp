#include "EnableKeypadLight.h"

// Configuration
#include "ListOfActions.h"

// Logging
#include "Logger.h"

EnableKeypadLight::EnableKeypadLight(VariableStore &store, HmiData &data) :
  Action(ENABLE_KEYPAD_VERB, ENABLE_KEYPAD_NOUN, data, ACTION_SINGLE, "KEYPAD ON"),
  store_(store)
{
}

void EnableKeypadLight::operation()
{
  LogTrace << "Running Keypad Light ON" << std::endl;

  /* set() persists the value and triggers the apply callback that
   * sends the keylight message to dskyKeyInt. Do NOT throw on failure:
   * an exception escaping operation() is uncaught in the action thread
   * and would terminate the process, so log the error instead. */
  if (!store_.set(VAR_NOUN_KEYPAD_LIGHT, 1))
  {
    LogError << "Failed to enable keypad light" << std::endl;
  }
}
