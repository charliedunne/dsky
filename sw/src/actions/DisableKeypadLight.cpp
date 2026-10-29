#include "DisableKeypadLight.h"

// Configuration
#include "ListOfActions.h"

// Logging
#include "Logger.h"

DisableKeypadLight::DisableKeypadLight(VariableStore &store, HmiData &data) :
  Action(DISABLE_KEYPAD_VERB, DISABLE_KEYPAD_NOUN, data, ACTION_SINGLE, "KEYPAD OFF"),
  store_(store)
{
}

void DisableKeypadLight::operation()
{
  LogTrace << "Running Keypad Light OFF" << std::endl;

  /* set() persists the value and triggers the apply callback that
   * sends the keylight message to dskyKeyInt. Do NOT throw on failure:
   * an exception escaping operation() is uncaught in the action thread
   * and would terminate the process, so log the error instead. */
  if (!store_.set(VAR_NOUN_KEYPAD_LIGHT, 0))
  {
    LogError << "Failed to disable keypad light" << std::endl;
  }
}
