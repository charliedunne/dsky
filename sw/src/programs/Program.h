#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include <string>
#include <thread>
#include <atomic>
#include "HmiData.h"

/**
 * @brief Base class for background programs.
 *
 * A program is a long-running activity launched from the HMI (PRO key + ID)
 * that runs on its own thread, independently of VERB/Action execution. The
 * user can keep operating the unit while a program is active.
 *
 * The execution model mirrors Action: start() spawns a thread that calls
 * loop(), which invokes operation() repeatedly with a fixed period until
 * stop() is called. Unlike an Action, a program has no single/periodic
 * schedule distinction: it is inherently long-running and periodic.
 */
class Program {

  private:

  /**
   * @brief Thread executing the program
   */
  std::thread thread_;

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
   * @brief Start the program in the background.
   *
   * If a previous run has completed, its thread is reaped before starting
   * so the assignment below never targets a joinable thread.
   */
  void start();

  /**
   * @brief Stop the program and wait for its thread to finish.
   */
  void stop();

  /**
   * @brief Getter for running state
   *
   * @return true if the program is running, false otherwise
   */
  bool isRunning() const;

  /**
   * @brief Operation to execute on each cycle.
   *
   * Pure virtual: each concrete program implements its own work here.
   */
  virtual void operation() = 0;

  protected:
  /**
   * @brief Main loop executed by the thread.
   *
   * Calls operation() repeatedly, sleeping for period_ milliseconds between
   * cycles, until stop() clears running_.
   */
  void loop();
};

#endif /* _PROGRAM_H_ */
