#ifndef _HMILOGIC_H_
#define _HMILOGIC_H_

// STL
#include <vector>

// Core
#include "Event.h"
#include "StatusMachine.h"

// HMI 
#include "Hmi.h"

// Actions 
#include "ListOfActions.h"

class HmiLogic {

 private:

  /**
   * @brief Key events detected for this cycle
   */
  std::vector<Event> keyEvents_;

  /**
   * @brief status machine for the Hmi Logic
   */
  StatusMachine status_;

  /**
   * @brief List of actions registered
   */
  ListOfActions actions_;

 public:

  HmiLogic();
  virtual ~HmiLogic();

  /**
   * @brief Parse Key Event list and include them locally
   *
   * @param[in] events List of events 
   */
  void parseKeyEvents(std::vector<Event> events);

  /**
   * @brief Update the struct to pass to the Hmi
   */
  void updateHmiData(HmiData &data);

  /**
   * @brief This function is used to register actions in the
   * logic
   * 
   * @param action 
   */
  void registerAction(Action & action);

 private:

  void idle(HmiData &data);
  void verbInput(HmiData &data);
  void nounInput(HmiData &data);
  void run(HmiData &data);
  void error(HmiData &data);
};

#endif /* _HMILOGIC_H_ */
