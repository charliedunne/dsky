#ifndef _TEMPMONITOR_H_
#define _TEMPMONITOR_H_

#include <atomic>

#include "Action.h"

class TempMonitor : public Action {

 public:

  TempMonitor(HmiData & data);

  /**
   * @brief Handle a keypad event while the action is running.
   *
   * In normal mode +/- cycle through the sensor list and CLR starts NOUN
   * entry. In entry mode digits append to the pending value (max 2),
   * ENTER accepts it if in range, and CLR cancels the entry.
   *
   * @param key The keypad event to handle.
   * @return true if the key was handled by this action.
   */
  bool handleKey(EventId_t key) override;

 protected:

  void operation() override;

 private:

  /**
   * @brief Append a digit to the pending NOUN entry value.
   *
   * @param d Digit (0-9) to append.
   * @return true if the digit was accepted, false if two digits are already entered.
   */
  bool enterDigit(int d);

  std::atomic<int> selected_{0};      //!< Index of the currently selected sensor
  std::atomic<bool> entering_{false}; //!< True while a NOUN value is being entered
  std::atomic<int> entryValue_{0};    //!< Pending NOUN value under entry
  std::atomic<int> entryDigits_{0};   //!< Number of digits accepted so far
};

#endif /* _TEMPMONITOR_H_ */
