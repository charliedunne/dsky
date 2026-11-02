#include "StopBgProg.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

// Exceptions
#include "ExcInvalidCommand.h"

// STL
#include <string>

StopBgProg::StopBgProg(ListOfPrograms &programs, HmiData &data) : Action(STOP_BG_VERB, STOP_BG_NOUN, data, ACTION_SINGLE, "STOP PROG"), programs_(programs)
{
  /* The registered noun is -1 (accept any), but a program ID must always be
   * typed; force the HMI to collect one before dispatch */
  setNeedsTypedNoun(true);
}

void StopBgProg::operation()
{
  /* Capture the target ID immediately: after MODE_RUN completes the HMI
   * resets the right-LCD data, and this action runs on a worker thread */
  int id = hmiData_.getRightLcdData().nNoun;

  if (id == UNASIGNED)
  {
    /* No program ID was typed (e.g. ENTR pressed right away) */
    LogTrace << "STOP_BG: no program ID provided" << std::endl;
    hmiData_.lSetOpErr(DRAW_BLINK);
    return;
  }

  try
  {
    Program &p = programs_(id);

    if (!p.isRunning())
    {
      LogTrace << "STOP_BG: program " << id << " is not running" << std::endl;
      hmiData_.lSetOpErr(DRAW_BLINK);
      return;
    }

    LogTrace << "STOP_BG: stopping program " << id << std::endl;

    /* Blocking stop (joins the program thread); safe here because this runs
     * on the action worker thread, not the HMI loop */
    p.stop();

    LogTrace << "STOP_BG: program " << id << " stopped" << std::endl;
  }
  catch (ExcInvalidCommand &)
  {
    /* No program registered with this ID */
    LogTrace << "STOP_BG: unknown program ID " << id << std::endl;
    hmiData_.lSetOpErr(DRAW_BLINK);
  }
}
