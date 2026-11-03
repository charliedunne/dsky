#include "StopBgProg.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

// Exceptions
#include "ExcInvalidCommand.h"

StopBgProg::StopBgProg(ListOfPrograms &programs, HmiData &data) : Action(STOP_BG_VERB, STOP_BG_NOUN, data, ACTION_PERIODIC, "STOP PROG"), programs_(programs)
{
  setPeriod(1000);
}

void StopBgProg::start()
{
  /* Capture the target program ID selected by the user in the noun field */
  pendingStopId_ = hmiData_.getRightLcdData().nNoun;
}

void StopBgProg::operation()
{
  const int id = pendingStopId_.load();

  bool ok = true;

  if (id == UNASIGNED)
  {
    /* No program selected in the noun field */
    LogTrace << "STOP_BG: no program selected" << std::endl;
    ok = false;
  }
  else
  {
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
  }

  /* One-shot: clear the list display and finish. resetLeftLcdData() also
   * clears lOpErr, so re-arm the OP ERROR blink afterwards on the error path */
  hmiData_.resetLeftLcdData();
  if (!ok) {
    hmiData_.lSetOpErr(DRAW_BLINK);
  }

  finish();
}
