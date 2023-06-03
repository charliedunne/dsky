#include "ActionClock.h"

#include <iostream>

// STL
#include <ctime>

// Logging
#include "Logger.h"

ActionClock::ActionClock(HmiData &data) : Action(1, -1, data, ACTION_PERIODIC)
{
  setPeriod(1000);
}

void ActionClock::operation()
{
  const std::time_t now = std::time(nullptr);
  const std::tm calendarTime = *std::localtime(std::addressof(now));

  hmiData_.rSetCompActLabel(DRAW_ON);

  hmiData_.rSetnR1(calendarTime.tm_hour, DRAW_ON);
  hmiData_.rSetR1Label(DRAW_ON);

  hmiData_.rSetnR2(calendarTime.tm_min, DRAW_ON);
  hmiData_.rSetR2Label(DRAW_ON);

  hmiData_.rSetnR3(calendarTime.tm_sec, DRAW_ON);
  hmiData_.rSetR3Label(DRAW_ON);
}
