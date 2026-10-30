#include "ActionVerbList.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

ActionVerbList::ActionVerbList(HmiData &data, const ListOfActions &actions, const VariableStore &store)
: Action(ACTION_LIST_VERB, ACTION_LIST_NOUN, data, ACTION_SINGLE, "VERB LIST"), actions_(actions), store_(store)
{
}

void ActionVerbList::operation()
{
  // Build both pages once (lists are static at runtime):
  // page 0 = verb list, page 1 = variable list. The user flips between them with +/-.
  hmiData_.lSetVerbList(actions_.describe(), store_.describeVars());
}
