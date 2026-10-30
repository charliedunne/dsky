#include "SystemReboot.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

// STL
#include <chrono>
#include <string>
#include <thread>
#include <vector>

SystemReboot::SystemReboot(HmiData &data) : Action(REBOOT_VERB, REBOOT_NOUN, data, ACTION_SINGLE, "REBOOT")
{
  cancel_ = false;
}

bool SystemReboot::cancellable() const
{
  return true;
}

void SystemReboot::cancel()
{
  LogTrace << "System reboot cancelled by user" << std::endl;
  cancel_ = true;
}

void SystemReboot::operation()
{
  /* Restart the countdown from zero on every new invocation */
  cancel_ = false;

  LogTrace << "Starting system reboot countdown (" << COUNTDOWN_SECONDS << " s)" << std::endl;

  /* Blink the REBOOT alarm label as the cancel-window indicator. Sleep in
   * small increments so a cancel request is picked up within ~100 ms */
  hmiData_.lSetReboot(DRAW_BLINK);

  /* Enable R1 Label */
  hmiData_.rSetR1Label(DRAW_ON);

  for (int remaining = COUNTDOWN_SECONDS; remaining > 0 && !cancel_; --remaining)
  {
    /* Show the seconds remaining in R1 (10, 9, ... 1) */
    hmiData_.rSetnR1(remaining, DRAW_ON);

    for (int i = 0; i < 10 && !cancel_; ++i)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  if (cancel_)
  {
    LogTrace << "Reboot aborted before restart" << std::endl;
    hmiData_.lSetReboot(DRAW_OFF);
    return;
  }

  /* Countdown expired: reboot */
  LogTrace << "Countdown over, issuing systemctl reboot" << std::endl;

  /* Show the final zero in R1 before the system goes down */
  hmiData_.rSetnR1(0, DRAW_ON);

  /* Give the HMI a moment to render before the system goes down */
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  /* dsky runs as a regular user: the actual permission is granted by a
   * polkit rule for org.freedesktop.login1.reboot on the target host */
  int ret = std::system("systemctl reboot");
  LogTrace << "systemctl reboot returned " << ret << std::endl;

  /* If the reboot was refused (e.g. missing polkit rule), stop blinking
   * so the HMI is not left stuck on a fake alarm */
  if (ret != 0)
  {
    hmiData_.lSetReboot(DRAW_OFF);
    hmiData_.rSetnR1(0, DRAW_OFF);
  }
}
