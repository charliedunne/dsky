#include "Action.h"

// STL
#include <stdexcept>
#include <chrono>

// Test
#include <iostream>
#include <unistd.h>

// Logging
#include "Logger.h"

Action::Action(int verb, int noun, HmiData &data, ActionSchedule_e actionSchedule, const std::string& label)
: hmiData_(data), label_(label)
{

  /* Set internal configuration */
  verb_ = verb;
  noun_ = noun;
  actionSchedule_ = actionSchedule;   // was a self-assignment bug
  running_ = false;
  finished_ = false;
}

Action::~Action()
{
  /* Never destroy a joinable std::thread: that calls std::terminate.
   * A finished thread joins instantly; while powering off the systemctl
   * client exits quickly, well before the machine actually goes down */
  if (thread_.joinable())
  {
    thread_.join();
  }
}

void Action::setPeriod(int period)
{

  /* Set the period */
  period_ = period;

  /* Change the type to periodic in case it wasn't */
  actionSchedule_ = ACTION_PERIODIC;
}

int Action::getVerb() const
{

  return verb_;
}

int Action::getNoun() const
{

  return noun_;
}

const std::string& Action::getLabel() const
{
  return label_;
}

ActionSchedule_e Action::getSchedule()
{

  return actionSchedule_;
}

void Action::loop()
{
  if (actionSchedule_ == ACTION_SINGLE)
  {
    LogTrace << "Single Action" << std::endl;
    operation();
    finished_ = true;
    running_ = false;
  }
  else
  {
    while (isRunning() == true)
    {
      /* Get time when the thread must stops */
      const auto next = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(period_);
    
      operation();
      
      // Wait for next cycle.
      std::this_thread::sleep_until(next);
    }
  }
}

void Action::run()
{
  /* Reap a previous completed thread so the assignment below never
   * targets a joinable thread (which would call std::terminate) */
  if (thread_.joinable())
  {
    thread_.join();
  }

  /* Reset per-run state before the action becomes visible as running,
   * so key handlers never observe stale state from a previous run */
  start();

  /* Set flag to running */
  running_ = true;
  finished_ = false;

  /* Create and start the thread */
  thread_ = std::thread(&Action::loop, this);
}

void Action::stop()
{
  running_ = false;

  /**
   * @todo In case of an error there must be a way to also
   * abort the thread
   */
  /* Join only if a thread was actually started: joining a
   * non-joinable thread is undefined behaviour */
  if (thread_.joinable())
  {
    thread_.join();
    thread_.~thread();
  }

  /* Back to the fresh state so the action can be run again */
  finished_ = false;
}

bool Action::isRunning() 
{
  return running_;
}

bool Action::finished()
{
  return finished_;
}

bool Action::cancellable() const
{
  return false;
}

void Action::cancel()
{
  /* Default action is not cancellable */
}
