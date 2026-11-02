#ifndef _FGPROGRAM_H_
#define _FGPROGRAM_H_

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include "Program.h"
#include "Event.h"

/**
 * @brief Base class for foreground (modal) programs.
 *
 * A foreground program runs on its own worker thread and requires keypad
 * input while running. The HMI enters a dedicated modal mode
 * (MODE_PROG_RUN) and forwards every key press via pushKey(); the derived
 * operation() pulls keys with nextKey(), which blocks until a key arrives
 * or the program is stopped/cancelled.
 *
 * Cancellation: if cancellable() returns true, any key press cancels the
 * program (non-blocking cancel()); otherwise the key is forwarded to the
 * input queue. A cancellable derived operation() must therefore poll
 * nextKey() (or cancelled()) regularly so it observes the request.
 */
class FgProgram : public Program {

  private:

  /**
   * @brief Worker thread executing operation()
   */
  std::thread thread_;

  /**
   * @brief Pending keypad events forwarded by the HMI
   */
  std::deque<EventId_t> keyQueue_;

  /**
   * @brief Guards keyQueue_
   */
  std::mutex mutex_;

  /**
   * @brief Notifies nextKey() when a key arrives or the program ends
   */
  std::condition_variable cv_;

  /**
   * @brief Set by cancel()/stop() to release a blocked nextKey()
   */
  std::atomic<bool> cancelled_;

  /**
   * @brief Worker entry point: runs operation(), then clears the running
   * flag via the base (idempotent, non-blocking).
   */
  void loop();

  public:
  FgProgram(int id, HmiData &data, const std::string& label = "", int period = 0);

  virtual ~FgProgram();

  /**
   * @brief Start the program and spawn its worker thread.
   */
  virtual void start() override;

  /**
   * @brief Stop the program and join its worker thread. No-op if not running.
   */
  virtual void stop() override;

  /**
   * @brief Foreground programs are modal: they own the HMI while running.
   */
  virtual bool foreground() const override { return true; }

  /**
   * @brief Forward a keypad event to the running program (thread-safe).
   *
   * Called by the HMI in MODE_PROG_RUN for every key press.
   */
  void pushKey(EventId_t key);

  /**
   * @brief True if this program can be cancelled by any key press.
   */
  virtual bool cancellable() const { return false; }

  /**
   * @brief Cancel the program without blocking (never joins).
   */
  virtual void cancel();

  /**
   * @brief True after cancel()/stop() has been requested.
   */
  bool cancelled() const { return cancelled_; }

  protected:

  /**
   * @brief Block until a key is available, then deliver it.
   *
   * @param[out] key receives the delivered event id
   * @return true if a key was delivered, false if the program stopped or
   *         was cancelled with nothing pending
   */
  bool nextKey(EventId_t &key);
};

#endif /* _FGPROGRAM_H_ */
