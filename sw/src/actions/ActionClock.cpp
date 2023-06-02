#include "ActionClock.h"

#include <iostream>

// STL
#include <ctime>

// Logging
#include "Logger.h"

ActionClock::ActionClock(HmiData *data) : Action(1, -1, data, ACTION_PERIODIC)
{
  setPeriod(1000);
}

void ActionClock::operation()
{
  const std::time_t now = std::time(nullptr);
  const std::tm calendarTime = *std::localtime(std::addressof(now));

  LogDebug << "InActionClock: BEFORE nR1 = " << hmiData_->nR1 << std::endl;

  hmiData_->nR1 = calendarTime.tm_hour;
  hmiData_->nR1Mode = DRAW_ON;
  hmiData_->lR1Mode = DRAW_ON;

  hmiData_->nR2 = calendarTime.tm_min;
  hmiData_->nR1Mode = DRAW_ON;
  hmiData_->lR3Mode = DRAW_ON;

  hmiData_->nR3 = calendarTime.tm_sec;
  hmiData_->nR1Mode = DRAW_ON;
  hmiData_->lR3Mode = DRAW_ON;

  LogDebug << "InActionClock: AFTER nR1 = " << hmiData_->nR1 << std::endl;

  //std::cout << "hmiData Address = 0x" << std::hex << hmiData_-><< std::endl;

  LogTrace << "Time: " << hmiData_->nR1 << ":" << hmiData_->nR2 << ":" << hmiData_->nR3 << std::endl;
}
