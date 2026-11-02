#include "StopBgProg.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

// Exceptions
#include "ExcInvalidCommand.h"

// STL
#include <cstdio>
#include <string>
#include <vector>

StopBgProg::StopBgProg(ListOfPrograms &programs, HmiData &data) : Action(STOP_BG_VERB, STOP_BG_NOUN, data, ACTION_PERIODIC, "STOP PROG"), programs_(programs)
{
  setPeriod(1000);
}

void StopBgProg::start()
{
  /* Reset per-run state so every run starts with nothing selected */
  selected_ = -1;
  pendingStopId_ = -1;
}

bool StopBgProg::handleKey(EventId_t key)
{
  const std::vector<int> ids = programs_.runningIds();
  const int count = (int)ids.size();
  const int cur = selected_.load();

  if (key == E_KEY_PLUS) {
    if (count == 0) return false;
    selected_.store(cur < 0 ? 0 : (cur + 1) % count);
    updateScreens();
    return true;
  }

  if (key == E_KEY_MINUS) {
    if (count == 0) return false;
    selected_.store(cur < 0 ? count - 1 : (cur - 1 + count) % count);
    updateScreens();
    return true;
  }

  if (key == E_KEY_ENTR) {
    /* No selection yet, or the list shrank since the last key: ignore */
    if (count == 0 || cur < 0 || cur >= count) return false;
    pendingStopId_.store(ids[cur]);
    updateScreens();
    return true;
  }

  return false;
}

void StopBgProg::operation()
{
  const int id = pendingStopId_.load();

  if (id < 0) {
    /* No stop requested yet: just refresh the list display */
    updateScreens();
    return;
  }

  bool ok = true;

  try
  {
    Program &p = programs_(id);

    if (!p.isRunning())
    {
      LogTrace << "STOP_BG: program " << id << " is not running" << std::endl;
      ok = false;
    }
    else
    {
      LogTrace << "STOP_BG: stopping program " << id << std::endl;

      /* Blocking stop (joins the program thread); safe here because this runs
       * on the action worker thread, not the HMI loop */
      p.stop();

      LogTrace << "STOP_BG: program " << id << " stopped" << std::endl;
    }
  }
  catch (ExcInvalidCommand &)
  {
    /* No program registered with this ID */
    LogTrace << "STOP_BG: unknown program ID " << id << std::endl;
    ok = false;
  }

  /* Clear the list display before finishing. resetLeftLcdData() also clears
   * lOpErr, so re-arm the OP ERROR blink afterwards on the error path */
  hmiData_.resetLeftLcdData();
  if (!ok) {
    hmiData_.lSetOpErr(DRAW_BLINK);
  }

  finish();
}

void StopBgProg::updateScreens()
{
  const std::vector<int> ids = programs_.runningIds();
  const int count = (int)ids.size();

  std::vector<std::string> lines;

  if (count == 0) {
    lines.push_back("NONE");
  } else {
    /* describeRunning() entries pair 1:1 with runningIds() when non-empty */
    const std::vector<std::string> entries = programs_.describeRunning();
    lines.reserve(entries.size());
    char line[28];
    for (size_t i = 0; i < entries.size(); ++i) {
      const bool sel = (int)i == selected_.load();
      snprintf(line, sizeof(line), "%s%s%s", sel ? "*" : "", entries[i].c_str(), sel ? "*" : "");
      lines.emplace_back(line);
    }
  }

  hmiData_.resetLeftLcdData();
  hmiData_.lSetVerbListHeader(0, "RUNNING");
  hmiData_.lSetVerbList(lines, {});
}
