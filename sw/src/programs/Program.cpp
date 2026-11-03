#include "Program.h"

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
}

void Program::setPeriod(int period)
{

  /* Set the period */
  period_ = period;
}

int Program::getPeriod() const
{

  return period_;
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
  running_ = true;
}

void Program::stop()
{
  if (!running_)
  {
    return;
  }

  running_ = false;
}

bool Program::isRunning() const
{

  return running_;
}
