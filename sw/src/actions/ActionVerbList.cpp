#include "ActionVerbList.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

ActionVerbList::ActionVerbList(HmiData &data, const ListOfActions &actions)
: Action(ACTION_LIST_VERB, ACTION_LIST_NOUN, data, ACTION_SINGLE, "VERB LIST"), actions_(actions)
{
}

void ActionVerbList::operation()
{
  // Build the menu lines from every registered action and show them on the left panel
  hmiData_.lSetVerbList(actions_.describe());
}
