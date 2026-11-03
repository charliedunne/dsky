#ifndef _DBMONITORPROGRAM_H_
#define _DBMONITORPROGRAM_H_

#include "BgProgram.h"
#include "VariableStore.h"

/* Program id selected on the keypad to launch DBMONITOR */
#define DBMONITOR_PROGRAM 2

/* Default execution period in seconds, used when no persisted value exists */
#define DBMONITOR_DEFAULT_PERIOD 60

/**
 * @brief Background program with a user-configurable execution period.
 *
 * The period is stored in the variable store under NOUN VAR_NOUN_DBMONITOR_PERIOD
 * (in seconds). On every start() the current variable value is read and used
 * as the execution period, so a change made via VERB 9 takes effect on the
 * next trigger. If no value is available, DBMONITOR_DEFAULT_PERIOD is used.
 */
class DbMonitorProgram : public BgProgram {

  private:

  /**
   * @brief Reference to the variable store holding the period
   */
  VariableStore &store_;

  /**
   * @brief Counter of executed cycles, used only for tracing
   */
  int cycle_;

  public:
  DbMonitorProgram(HmiData &data, VariableStore &store);

  /**
   * @brief Start the program.
   *
   * Reads the period variable (seconds) from the store, converts it to
   * milliseconds and applies it via setPeriod(), then starts the worker
   * thread through BgProgram::start().
   */
  void start() override;

  /**
   * @brief Emit a heartbeat trace line (runs once per period).
   */
  void operation() override;
};

#endif /* _DBMONITORPROGRAM_H_ */
