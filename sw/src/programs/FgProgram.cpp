#include "FgProgram.h"

// Logging
#include "Logger.h"

FgProgram::FgProgram(int id, HmiData &data, const std::string& label, int period) :
    Program(id, data, label, period),
    cancelled_(false)
{
}

FgProgram::~FgProgram()
{
  /* Never destroy a joinable std::thread: that calls std::terminate.
   * A finished thread joins instantly. */
  if (thread_.joinable())
  {
    thread_.join();
  }
}

void FgProgram::start()
{
  /* Reap a previous completed thread so the assignment below never
   * targets a joinable thread (which would call std::terminate) */
  if (thread_.joinable())
  {
    thread_.join();
  }

  /* Reset cancellation state and drop any stale keys from a prior run */
  cancelled_ = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    keyQueue_.clear();
  }

  /* Mark running via the base, then spawn the worker thread */
  Program::start();
  thread_ = std::thread(&FgProgram::loop, this);
}

void FgProgram::stop()
{
  if (!isRunning())
  {
    return;
  }

  /* Clear the running flag via the base (non-blocking), release any
   * worker blocked in nextKey(), then join */
  Program::stop();
  cancelled_ = true;
  cv_.notify_all();
  if (thread_.joinable())
  {
    thread_.join();
  }
}

void FgProgram::pushKey(EventId_t key)
{
  {
    std::lock_guard<std::mutex> lock(mutex_);
    keyQueue_.push_back(key);
  }
  cv_.notify_all();
}

void FgProgram::cancel()
{
  /* Non-blocking: only request cancellation and release a worker blocked
   * in nextKey(). The worker clears the running flag itself on exit. */
  cancelled_ = true;
  cv_.notify_all();
}

bool FgProgram::nextKey(EventId_t &key)
{
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock, [this] {
    return !keyQueue_.empty() || !isRunning() || cancelled_;
  });

  if (!keyQueue_.empty())
  {
    key = keyQueue_.front();
    keyQueue_.pop_front();
    return true;
  }

  /* Stopped or cancelled with nothing pending */
  return false;
}

void FgProgram::loop()
{
  LogTrace << "Program " << getId() << " started" << std::endl;

  operation();

  /* Clear the running flag via the base (idempotent, non-blocking) so a
   * concurrent stop() join never waits on us */
  Program::stop();

  LogTrace << "Program " << getId() << " stopped" << std::endl;
}
