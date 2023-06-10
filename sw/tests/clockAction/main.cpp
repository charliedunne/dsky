#include "ActionClock.h"

#include <iostream>
#include <signal.h>

void signal_callback_handler(int signum, ActionClock *a)
{
  static ActionClock *ptr = nullptr;

  if (ptr == NULL)
  {
    ptr = a;
  }

  if (signum == SIGINT)
  {

    std::cout << "System Signal received (" << signum << ")." << std::endl;
    ptr->stop();
  }
}

int main()
{

  HmiData data;

  ActionClock clockAction(&data);

  // Register handler for SIGNING
  signal(SIGINT, (void (*)(int))signal_callback_handler);
  signal_callback_handler(0, &clockAction);

  clockAction.setPeriod(1000);

  clockAction.run();
  return 0;
}
