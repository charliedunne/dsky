#include "DbMonitorProgram.h"

// Logging
#include "Logger.h"

// Noun selector for the period variable (VERB 8/9)
#include "ListOfActions.h"

// DB client whose health is reported on the TEMP alarm label
#include "DbClient.h"

/* Minimum period in seconds, guards against a zero/negative stored value */
#define DBMONITOR_MIN_PERIOD 1

DbMonitorProgram::DbMonitorProgram(HmiData &data, VariableStore &store, DbClient &db) :
    BgProgram(DBMONITOR_PROGRAM, data, "DBMONITOR", DBMONITOR_DEFAULT_PERIOD * 1000),
    store_(store), db_(db), cycle_(0)
{
}

void DbMonitorProgram::start()
{
  /* Read the period variable (seconds) and apply it to this run. The read
   * happens on the HMI thread; the worker thread only ever sees the cached
   * period_ member, so no store access is needed from the worker. */
  int seconds = DBMONITOR_DEFAULT_PERIOD;
  if (!store_.get(VAR_NOUN_DBMONITOR_PERIOD, seconds) || seconds < DBMONITOR_MIN_PERIOD) {
    seconds = DBMONITOR_DEFAULT_PERIOD;
  }
  setPeriod(seconds * 1000);

  BgProgram::start();
}

void DbMonitorProgram::operation()
{
  /* Blink the TEMP alarm while the database is not accessible, turn it off
   * when reachable. HmiData setters are mutex-guarded, so calling them from
   * this worker thread is safe. */
  hmiData_.lSetTemp(db_.isHealthy() ? DRAW_OFF : DRAW_BLINK);

  cycle_++;
  LogTrace << "DbMonitorProgram heartbeat, cycle " << cycle_
           << ", db " << (db_.isHealthy() ? "ok" : "DOWN") << std::endl;
}
