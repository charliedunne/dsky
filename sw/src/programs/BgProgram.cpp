#include "BgProgram.h"

// STL
#include <algorithm>
#include <chrono>

// Logging
#include "Logger.h"

BgProgram::BgProgram(int id, HmiData &data, const std::string& label, int period) :
    Program(id, data, label, period)
{
}

BgProgram::~BgProgram()
{
  /* Never destroy a joinable std::thread: that calls std::terminate.
   * A finished thread joins instantly. */
  if (thread_.joinable())
  {
    thread_.join();
  }
}

void BgProgram::start()
{
  /* Defensive guard: if already running, joining the still-running worker
   * would block forever. The HMI normally rejects re-triggering, but this
   * keeps start() safe for any caller */
  if (isRunning())
  {
    LogTrace << "BgProgram::start ignored, already running [id: " << getId() << "]" << std::endl;
    return;
  }

  /* Reap a previous completed thread so the assignment below never
   * targets a joinable thread (which would call std::terminate) */
  if (thread_.joinable())
  {
    thread_.join();
  }

  /* Mark running via the base, then spawn the worker thread */
  Program::start();
  thread_ = std::thread(&BgProgram::loop, this);
}

void BgProgram::stop()
{
  if (!isRunning())
  {
    return;
  }

  /* Clear the running flag via the base, then join the worker */
  Program::stop();
  if (thread_.joinable())
  {
    thread_.join();
  }
}

void BgProgram::loop()
{
  LogTrace << "Program " << getId() << " started" << std::endl;

  while (isRunning())
  {
    /* Get time when the thread must stop */
    const auto next = std::chrono::steady_clock::now() +
                      std::chrono::milliseconds(getPeriod());

    operation();

    // Wait for next cycle in short slices so stop() is responsive:
    // a single full-period sleep would delay shutdown by up to one period.
    if (getPeriod() > 0)
    {
      const auto slice = std::chrono::milliseconds(100);
      auto now = std::chrono::steady_clock::now();
      while (isRunning() && now < next)
      {
        std::this_thread::sleep_until(std::min(now + slice, next));
        now = std::chrono::steady_clock::now();
      }
    }
  }

  LogTrace << "Program " << getId() << " stopped" << std::endl;
}
