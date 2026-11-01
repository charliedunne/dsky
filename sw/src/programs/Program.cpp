#include "Program.h"

// STL
#include <chrono>

// Logging
#include "Logger.h"

Program::Program(int id, HmiData &data, const std::string& label, int period) :
    hmiData_(data), label_(label)
{

  /* Set internal configuration */
  id_ = id;
  period_ = period;
  running_ = false;
}

Program::~Program()
{
  /* Never destroy a joinable std::thread: that calls std::terminate.
   * A finished thread joins instantly. */
  if (thread_.joinable())
  {
    thread_.join();
  }
}

void Program::setPeriod(int period)
{

  /* Set the period */
  period_ = period;
}

int Program::getId() const
{

  return id_;
}

const std::string& Program::getLabel() const
{

  return label_;
}

void Program::start()
{
  /* Reap a previous completed thread so the assignment below never
   * targets a joinable thread (which would call std::terminate) */
  if (thread_.joinable())
  {
    thread_.join();
  }

  running_ = true;
  thread_ = std::thread(&Program::loop, this);
}

void Program::stop()
{
  if (!running_)
  {
    return;
  }

  running_ = false;
  if (thread_.joinable())
  {
    thread_.join();
  }
}

bool Program::isRunning() const
{

  return running_;
}

void Program::loop()
{
  LogTrace << "Program " << id_ << " started" << std::endl;

  while (running_)
  {
    /* Get time when the thread must stop */
    const auto next = std::chrono::steady_clock::now() +
                      std::chrono::milliseconds(period_);

    operation();

    // Wait for next cycle.
    if (period_ > 0)
    {
      std::this_thread::sleep_until(next);
    }
  }

  LogTrace << "Program " << id_ << " stopped" << std::endl;
}
