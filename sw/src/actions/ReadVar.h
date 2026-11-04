#ifndef _READVAR_H_
#define _READVAR_H_

#include <atomic>

#include "Action.h"
#include "VariableStore.h"

/**
 * @brief VERB 8: read and display DSKY variables.
 *
 * Verb-only periodic action (no typed noun required). Keeps the left LCD
 * populated with the list of all known variables (asterisk marks the selected
 * one) and shows the selected variable's NOUN and current value on the right
 * LCD. +/- cycles the selection, so the user can browse every variable without
 * re-entering the command; ENTER or RSET ends the action.
 */
class ReadVar : public Action {

 public:

  /**
   * @brief Constructor for the READ_VAR action.
   *
   * @param store Persistent variable store (must outlive this action).
   * @param hmi Shared HMI data (not owned).
   */
  ReadVar(VariableStore &store, HmiData &hmi);

  /**
   * @brief Handle a keypad event while the action is running.
   *
   * +/- cycle through the variable list; every other key is ignored.
   *
   * @param key The keypad event to handle.
   * @return true if the key was handled by this action.
   */
  bool handleKey(EventId_t key) override;

 protected:

  /** @brief Draw initial screens and start the periodic cycle. */
  void start() override;

  /** @brief Redraw both LCDs to reflect the current selection. */
  void operation() override;

 private:

  /**
   * @brief Redraw both LCD screens from the current selection.
   *
   * Called by operation() on every period tick and by handleKey() after any
   * accepted key, so the display reacts to keypresses immediately instead of
   * waiting for the next tick.
   */
  void updateScreens();

  VariableStore &store_;      //!< Persistent variable store (not owned)
  std::atomic<int> selected_{0}; //!< Index into VariableStore::varList()
};

#endif /* _READVAR_H_ */
