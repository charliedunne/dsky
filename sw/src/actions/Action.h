#ifndef _ACTION_H_
#define _ACTION_H_

/* HMI Definitions */
#include "HmiData.h"

/* STL */
#include <thread>
#include <functional>

typedef enum
{

  ACTION_SINGLE,
  ACTION_PERIODIC,
  ACTION_SPORADIC

} ActionSchedule_e;

class Action
{

private:
  /**
   * @brief thread object
   */
  std::thread thread_;
  /**
   * @brief Action Type (in terms of scheduling)
   */
  ActionSchedule_e actionSchedule_;

  /**
   * @brief Period of the cases of PERIODIC Actions [in milliseconds]
   */
  int period_ = 0;

  /**
   * @brief identifier of the action Verb (what to do)
   */
  int verb_ = -1;

  /**
   * @brief identifier of the target action (to what)
   */
  int noun_ = -1;

  /**
   * @brief Flag to determine if the thread must continue its execution
   *
   */
  std::atomic_bool running_;

  /**
   * @brief Flag to determine if the operation has finished.
   * 
   */
  std::atomic_bool finished_;

protected:
  HmiData &hmiData_;

public:
  Action(int verb, int noun, HmiData &data, ActionSchedule_e actionSchedule = ACTION_SINGLE);

  virtual ~Action();

  /**
   * @brief Set the Period in case of PERIODIC Scheduler
   *
   * @param[in] Period (in milliseconds)
   */
  void setPeriod(int period);

  /**
   * @brief Getter for Verb
   *
   * @return Verb
   */
  int getVerb();

  /**
   * @brief Getter for Noun
   *
   * @return Noun
   */
  int getNoun();

  /**
   * @brief Getter for Action Schedule type
   *
   * @return Action Schedule type
   */
  ActionSchedule_e getSchedule();

  /**
   * @brief Execute action.
   *
   * @note Note that this method will run the virtual function @b thread()
   * in the way as it is specified by the Schedule type.
   */
  void run();

  /**
   * @brief Command from the exterior the stop the operation
   */
  void stop();

  /**
   * @brief Check the execution status of the action
   *
   * @return true Action is being executed right now
   * @return false Action is not being executed.
   */
  bool isRunning();

  /**
   * @brief Check if the action has been marked as finisehd
   * 
   * @return true If the action has been marked as finished
   * @return false Otherwise
   */
  bool finished();

protected:
  /**
   * @brief Define in derived classes the operation to do in the thread
   */
  virtual void operation() = 0;

  void loop();
};

#endif /* _ACTION_H_ */
