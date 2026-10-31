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

  /**
   * @brief Reset per-run state at the start of every run.
   *
   * Clears any selection or pending NOUN entry left over from a
   * previous run so VERB 10 always starts with nothing selected.
   */
  void start() override;

  void operation() override;

 private:

  /**
   * @brief Append a digit to the pending NOUN entry value.
   *
   * @param d Digit (0-9) to append.
   * @return true if the digit was accepted, false if two digits are already entered.
   */
  bool enterDigit(int d);

  /**
   * @brief Redraw both LCD screens from the current state.
   *
   * Called by operation() on every period tick and by handleKey() after any
   * accepted key, so the display reacts to keypresses immediately instead of
   * waiting for the next tick.
   */
  void updateScreens();

  std::atomic<int> selected_{-1};     //!< Index of the currently selected sensor (-1 = none)
  std::atomic<bool> entering_{false}; //!< True while a NOUN value is being entered
  std::atomic<int> entryValue_{0};    //!< Pending NOUN value under entry
  std::atomic<int> entryDigits_{0};   //!< Number of digits accepted so far
};

#endif /* _TEMPMONITOR_H_ */
