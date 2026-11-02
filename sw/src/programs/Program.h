#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include <string>
#include <atomic>
#include "HmiData.h"

/**
 * @brief Base class for all programs.
 *
 * A program is an activity launched from the HMI (PRO key + ID). The base
 * class only holds identity, period and run state; it does not own a worker
 * thread. Concrete execution models are provided by the derived classes:
 *  - BgProgram: periodic background loop on its own thread, no user input.
 *  - FgProgram: modal, user-driven, requires keypad input while running.
 *
 * start()/stop() are virtual (non-pure) so ListOfPrograms::stopAll() can drive
 * every program uniformly at shutdown without touching a half-destroyed object.
 */
class Program {

  private:

  /**
   * @brief Program identifier (selected on the keypad)
   */
  int id_;

  /**
   * @brief Execution period in milliseconds (0 = as fast as possible)
   */
  int period_;

  /**
   * @brief Human-readable label shown on screen while running
   */
  std::string label_;

  /**
   * @brief Flag to determine if the program is running.
   * 
   */
  std::atomic<bool> running_;

  protected:
  HmiData &hmiData_;

  public:
  Program(int id, HmiData &data, const std::string& label = "", int period = 0);

  virtual ~Program();

  /**
   * @brief Set the execution period in milliseconds.
   *
   * @param[in] Period (in milliseconds)
   */
  void setPeriod(int period);

  /**
   * @brief Getter for the execution period in milliseconds.
   *
   * @return Period (in milliseconds)
   */
  int getPeriod() const;

  /**
   * @brief Getter for the program id
   *
   * @return Program id
   */
  int getId() const;

  /**
   * @brief Getter for the human-readable label
   *
   * @return Label
   */
  const std::string& getLabel() const;

  /**
   * @brief Start the program.
   *
   * The base implementation only marks the program as running. Derived classes
   * that own a worker thread override this to spawn it. Kept virtual (non-pure)
   * so ListOfPrograms::stopAll() can drive every program uniformly.
   */
  virtual void start();

  /**
   * @brief Stop the program and wait for it to finish.
   *
   * The base implementation only clears the running flag. Derived classes that
   * own a worker thread override this to join it. No-op if not running.
   */
  virtual void stop();

  /**
   * @brief Check whether the program is currently running
   *
   * @return True if running, false otherwise
   */
  bool isRunning() const;

  /**
   * @brief Whether this program requires user input while running.
   *
   * Foreground programs run modally: the HMI forwards keypad events to them
   * until they finish. Background programs keep the HMI free. Default false.
   */
  virtual bool foreground() const { return false; }

  /**
   * @brief Operation executed while the program is running.
   *
   * Derived classes implement their behaviour here. How it is driven (once per
   * period on a worker thread, or once per user input) depends on the concrete
   * execution model.
   */
  virtual void operation() = 0;
};

#endif /* _PROGRAM_H_ */
