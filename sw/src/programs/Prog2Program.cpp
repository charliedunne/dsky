#include "Prog2Program.h"

// Logging
#include "Logger.h"

// Noun selector for the period variable (VERB 8/9)
#include "ListOfActions.h"

/* Minimum period in seconds, guards against a zero/negative stored value */
#define PROG2_MIN_PERIOD 1

Prog2Program::Prog2Program(HmiData &data, VariableStore &store) :
    BgProgram(PROG2_PROGRAM, data, "PROG2", PROG2_DEFAULT_PERIOD * 1000),
    store_(store), cycle_(0)
{
}

void Prog2Program::start()
{
  /* Read the period variable (seconds) and apply it to this run. The read
   * happens on the HMI thread; the worker thread only ever sees the cached
   * period_ member, so no store access is needed from the worker. */
  int seconds = PROG2_DEFAULT_PERIOD;
  if (!store_.get(VAR_NOUN_PROG2_PERIOD, seconds) || seconds < PROG2_MIN_PERIOD) {
    seconds = PROG2_DEFAULT_PERIOD;
  }
  setPeriod(seconds * 1000);

  BgProgram::start();
}

void Prog2Program::operation()
{
  /* Do nothing but report a heartbeat so the background execution can be
   * observed in the journal while VERB actions run concurrently. */
  cycle_++;
  LogTrace << "Prog2Program heartbeat, cycle " << cycle_ << std::endl;
}
