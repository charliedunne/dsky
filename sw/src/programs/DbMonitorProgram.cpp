#include "DbMonitorProgram.h"

// Logging
#include "Logger.h"

// Noun selector for the period variable (VERB 8/9)
#include "DskyVariables.h"

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
  if (!store_.get(DskyVariables::NOUN_DBMONITOR_PERIOD, seconds) || seconds < DBMONITOR_MIN_PERIOD) {
    seconds = DBMONITOR_DEFAULT_PERIOD;
  }
  setPeriod(seconds * 1000);

  BgProgram::start();
}

void DbMonitorProgram::operation()
{
  /* Publish the database health condition. The TEMP alarm label is derived
   * from this condition every frame by HmiLogic, so the blink persists across
   * RSET wipes instead of waiting for the next monitor cycle. */
  hmiData_.setDbHealthy(db_.isHealthy());

  cycle_++;
  LogTrace << "DbMonitorProgram heartbeat, cycle " << cycle_
           << ", db " << (db_.isHealthy() ? "ok" : "DOWN") << std::endl;
}
