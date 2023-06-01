#ifndef _ACTION_H_
#define _ACTION_H_

/* HMI Definitions */
#include "HmiDefinitions.h"

/* STL */
#include <thread>
#include <functional>

typedef enum {

  ACTION_SINGLE,
  ACTION_PERIODIC,
  ACTION_SPORADIC

} ActionSchedule_e;

class Action {

 private:

  /**
   * @brief thread object
   */
  std::thread * thread_ = NULL;
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
   bool running_;


 protected:

  /**
   * @brief pointer to the Data to be updated (HMI Data input)
   */
  HmiData * hmiData_ = NULL;

 public:

  Action(int verb, int noun, HmiData * data, ActionSchedule_e actionSchedule = ACTION_SINGLE);

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

 protected:

  /**
   * @brief Define in derived classes the operation to do in the thread
   */
  virtual void operation() = 0;

  void loop();
};

#endif /* _ACTION_H_ */
