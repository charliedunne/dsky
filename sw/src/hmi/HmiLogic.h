#ifndef _HMILOGIC_H_
#define _HMILOGIC_H_

// STL
#include <chrono>
#include <vector>

// Core
#include "Event.h"
#include "StatusMachine.h"
#include "VariableStore.h"

// HMI 
#include "Hmi.h"

// Actions 
#include "ListOfActions.h"

// Programs
#include "ListOfPrograms.h"
#include "FgProgram.h"

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

  /**
   * @brief List of programs registered (background and foreground)
   */
  ListOfPrograms programs_;

  /**
   * @brief Foreground program currently owning the HMI (MODE_PROG_RUN)
   */
  FgProgram *fgActive_ = nullptr;

  /**
   * @brief True while a background program start is waiting out its downcount
   */
  bool bgCountdownActive_ = false;

  /**
   * @brief ID of the background program scheduled to start when the count hits zero
   */
  int bgCountdownProgId_ = 0;

  /**
   * @brief Time at which the background program downcount started
   */
  std::chrono::steady_clock::time_point bgCountdownStart_;

  /**
   * @brief Persistent variable store (used to validate bool digit input)
   */
  VariableStore &store_;

 public:

  /**
   * @brief Construct the HMI logic bound to a variable store.
   * @param store Store used to query variable types during value input
   */
  HmiLogic(VariableStore &store);
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

  /**
   * @brief Access the registered actions (used by VERB 0 to list them)
   */
  ListOfActions &getActions();

  /**
   * @brief Register a background program in the logic
   *
   * @param program Program to register
   */
  void registerProgram(Program &program);

  /**
   * @brief Access the registered background programs
   */
  ListOfPrograms &getPrograms();

 private:

  void idle(HmiData &data);
  void verbInput(HmiData &data);
  void nounInput(HmiData &data);
  void valueInput(HmiData &data);
  void programInput(HmiData &data);
  void run(HmiData &data);
  void programRun(HmiData &data);
  void error(HmiData &data);

  /**
   * @brief Advance the background-program start downcount by one frame.
   *
   * While active, updates R1 with the remaining whole seconds and starts the
   * scheduled program once the count reaches zero.
   */
  void bgCountdownTick(HmiData &data);
};

#endif /* _HMILOGIC_H_ */
