#ifndef _DUMMYPROGRAM_H_
#define _DUMMYPROGRAM_H_

#include "Program.h"

/* Program id selected on the keypad to launch the dummy program */
#define DUMMY_PROGRAM 1

/**
 * @brief Dummy background program used to validate the PROGRAM framework.
 *
 * Does nothing but emit a trace line to the journal every cycle (5 s), so
 * the heartbeat can be observed while confirming that VERB actions still run
 * concurrently in the foreground.
 */
class DummyProgram : public Program {

  private:

  /**
   * @brief Counter of executed cycles, used only for tracing
   */
  int cycle_;

  public:
  DummyProgram(HmiData &data);

  virtual ~DummyProgram();

  /**
   * @brief Emit a heartbeat trace line (runs once per period).
   */
  void operation() override;
};

#endif /* _DUMMYPROGRAM_H_ */
