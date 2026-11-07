#include "ListBgProg.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

// STL
#include <string>

ListBgProg::ListBgProg(ListOfPrograms &programs, HmiData &data) : Action(LIST_BG_VERB, LIST_BG_NOUN, data, ACTION_SINGLE, "LIST BG"), programs_(programs)
{
}

void ListBgProg::operation()
{
  LogTrace << "LIST_BG: listing running background programs" << std::endl;

  // Build the list of currently running programs
  std::vector<std::string> lines = programs_.describeRunning();

  // Display on the left LCD: reset, then show the program-list view (no selection)
  hmiData_.resetLeftLcdData();
  hmiData_.showProgList(lines, -1, "LIST OF RUNNING PROGRAMS");
}
