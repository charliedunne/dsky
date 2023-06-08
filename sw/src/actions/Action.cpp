#include "Action.h"

// STL
#include <stdexcept>
#include <chrono>

// Test
#include <iostream>
#include <unistd.h>

// Logging
#include "Logger.h"

Action::Action(int verb, int noun, HmiData &data, ActionSchedule_e actionSchedule) 
: hmiData_(data)
{

  /* Set internal configuration */
  verb_ = verb;
  noun_ = noun;
  actionSchedule_ = actionSchedule_;
  running_ = false;

  //std::cout << "hmiDataPtr = 0x" << std::hex << hmiData_ << std::endl;
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
  /* Set flag to running */
  running_ = true;

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
  thread_.join();

  if (actionSchedule_ != ACTION_SINGLE)
  {
    thread_.~thread();
  }
}

bool Action::isRunning() 
{
  return running_;
}