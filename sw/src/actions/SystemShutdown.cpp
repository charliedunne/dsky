#include "SystemShutdown.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

// STL
#include <chrono>
#include <string>
#include <thread>
#include <vector>

SystemShutdown::SystemShutdown(HmiData &data) : Action(SHUTDOWN_VERB, SHUTDOWN_NOUN, data, ACTION_SINGLE, "SHUTDOWN")
{
  cancel_ = false;
}

bool SystemShutdown::cancellable() const
{
  return true;
}

void SystemShutdown::cancel()
{
  LogTrace << "System shutdown cancelled by user" << std::endl;
  cancel_ = true;
}

void SystemShutdown::operation()
{
  /* Restart the countdown from zero on every new invocation */
  cancel_ = false;

  LogTrace << "Starting system shutdown countdown (" << COUNTDOWN_SECONDS << " s)" << std::endl;

  /* Blink the SHUTDOWN alarm label as the cancel-window indicator. Sleep in
   * small increments so a cancel request is picked up within ~100 ms */
  hmiData_.lSetShutdown(DRAW_BLINK);

  for (int remaining = COUNTDOWN_SECONDS; remaining > 0 && !cancel_; --remaining)
  {
    for (int i = 0; i < 10 && !cancel_; ++i)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  if (cancel_)
  {
    LogTrace << "Shutdown aborted before poweroff" << std::endl;
    hmiData_.lSetShutdown(DRAW_OFF);
    return;
  }

  /* Countdown expired: power off */
  LogTrace << "Countdown over, issuing systemctl poweroff" << std::endl;

  /* Give the HMI a moment to render before the system goes down */
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  /* dsky runs as a regular user: the actual permission is granted by a
   * polkit rule for org.freedesktop.login1.power-off on the target host */
  int ret = std::system("systemctl poweroff");
  LogTrace << "systemctl poweroff returned " << ret << std::endl;

  /* If the poweroff was refused (e.g. missing polkit rule), stop blinking
   * so the HMI is not left stuck on a fake alarm */
  if (ret != 0)
  {
    hmiData_.lSetShutdown(DRAW_OFF);
  }
}
