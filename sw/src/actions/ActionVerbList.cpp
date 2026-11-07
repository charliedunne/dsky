#include "ActionVerbList.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

ActionVerbList::ActionVerbList(HmiData &data, const ListOfActions &actions, const VariableStore &store, const ListOfPrograms &programs)
: Action(ACTION_LIST_VERB, ACTION_LIST_NOUN, data, ACTION_SINGLE, "VERB LIST"), actions_(actions), store_(store), programs_(programs)
{
}

void ActionVerbList::operation()
{
  // Build all three pages once (lists are static at runtime):
  // page 0 = verb list, page 1 = variable list, page 2 = program list.
  // The user flips between them with +/-.
  hmiData_.showVerbList(actions_.describe(), store_.describeVars(), programs_.describe());
}
