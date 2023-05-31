#include "ActionClock.h"

#include <iostream>

ActionClock::ActionClock(HmiData * data) : Action(1, -1, data, ACTION_PERIODIC) {

  /* Base constructor */
  //  super(1, -1, data, ACTION_PERIODIC);
}

void ActionClock::operation() {

  static int i = 0;

  hmiData_->nR1 = i++;
  hmiData_->nR1Mode = DRAW_ON;

  std::cout << "i = " << i << std::endl;
}
