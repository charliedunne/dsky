#include "DummyProgram.h"

// Logging
#include "Logger.h"

/* Heartbeat period in milliseconds */
#define DUMMY_PROGRAM_PERIOD 5000

DummyProgram::DummyProgram(HmiData &data) :
    Program(DUMMY_PROGRAM, data, "DUMMY", DUMMY_PROGRAM_PERIOD), cycle_(0)
{
}

DummyProgram::~DummyProgram()
{
}

void DummyProgram::operation()
{
  /* Do nothing but report a heartbeat so the background execution can be
   * observed in the journal while VERB actions run concurrently. */
  cycle_++;
  LogTrace << "DummyProgram heartbeat, cycle " << cycle_ << std::endl;
}
