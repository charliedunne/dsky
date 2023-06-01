#include "ActionClock.h"

#include <iostream>

// STL
#include <ctime>

ActionClock::ActionClock(HmiData *data) : Action(1, -1, data, ACTION_PERIODIC)
{

  /* Base constructor */
  //  super(1, -1, data, ACTION_PERIODIC);
}

void ActionClock::operation()
{

  const std::time_t now = std::time(nullptr);
  const std::tm calendarTime = *std::localtime(std::addressof(now));

  hmiData_->nR1 = calendarTime.tm_hour;
  hmiData_->nR1Mode = DRAW_ON;

  hmiData_->nR2 = calendarTime.tm_min;
  hmiData_->nR1Mode = DRAW_ON;

  hmiData_->nR3 = calendarTime.tm_sec;
  hmiData_->nR1Mode = DRAW_ON;

  std::cout << "Time: " << hmiData_->nR1 << ":" << hmiData_->nR2 << ":" << hmiData_->nR3 << std::endl;
}
