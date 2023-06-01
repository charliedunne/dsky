#include "Action.h"

// STL
#include <stdexcept>

// Test
#include <iostream>
#include <unistd.h>

Action::Action(int verb, int noun, HmiData * data, ActionSchedule_e actionSchedule) {

  /* Set internal configuration */
  verb_ = verb;
  noun_ = noun;
  hmiData_ = data;
  actionSchedule_ = actionSchedule_;
  running_ = true;
}

Action::~Action() {
}

void Action::setPeriod(int period) {

  /* Set the period */
  period_ = period;

  /* Change the type to periodic in case it wasn't */
  actionSchedule_ = ACTION_PERIODIC;
}

int Action::getVerb() {

  return verb_;
}

int Action::getNoun() {

  return noun_;
}

ActionSchedule_e Action::getSchedule() {

  return actionSchedule_;
}

void Action::loop() {

  if (actionSchedule_ == ACTION_SINGLE) {
    std::cout << "Action Single" << std::endl;
    operation();
  }
  else {
    while (running_) {
      std::cout << "Periodic" << std::endl;
      operation();
      usleep(1000 * period_);
    }
  }
}

void Action::run() {

  /* Create and start the thread */
  thread_ = new std::thread(&Action::loop, this);

  /* Wait for the thread to finish */
  thread_->join();
}

void Action::stop() {

  running_ = false;

  /**
   * @todo In case of an error there must be a way to also
   * abort the thread
   */
}
