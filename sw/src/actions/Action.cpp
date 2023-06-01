#include "Action.h"

// STL
#include <stdexcept>
#include <chrono>

// Test
#include <iostream>
#include <unistd.h>

// Logging
#include "Logger.h"

Action::Action(int verb, int noun, HmiData *data, ActionSchedule_e actionSchedule)
{

  /* Set internal configuration */
  verb_ = verb;
  noun_ = noun;
  hmiData_ = data;
  actionSchedule_ = actionSchedule_;
  running_ = false;
}

Action::~Action()
{
}

void Action::setPeriod(int period)
{

  /* Set the period */
  period_ = period;

  /* Change the type to periodic in case it wasn't */
  actionSchedule_ = ACTION_PERIODIC;
}

int Action::getVerb()
{

  return verb_;
}

int Action::getNoun()
{

  return noun_;
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
  }
  else
  {
    while (this->running_)
    {
      /* Get time when the thread must stops */
      const auto next = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(period_);
    
      operation();

      LogDebug << "Inside thread while, running_ = " << this->running_ << std::endl;
      
      // Wait for next cycle.
      std::this_thread::sleep_until(next);
    }

    LogDebug << "Thread finished" << std::endl;
  }
}

void Action::run()
{
  /* Set flag to running */
  running_ = true;

  /* Create and start the thread */
  thread_ = new std::thread(&Action::loop, this);

  /* Wait for the thread to finish */
  //thread_->join();
}

void Action::stop()
{

  LogDebug << "Before Stop thread commanded, runnin_ = " << running_ << std::endl;
  running_ = false;
  LogDebug << "After Stop thread commanded, runnin_ = " << running_ << std::endl;

  /**
   * @todo In case of an error there must be a way to also
   * abort the thread
   */
}

bool Action::isRunning() 
{
  return running_;
}