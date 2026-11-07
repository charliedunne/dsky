#pragma once

#include <atomic>

#include "Action.h"

class VariableStore;

/**
 * @brief VERB 9: write a value to a variable after confirmation.
 *
 * Verb-only action (no typed noun required). Runs in two phases:
 *
 *  - Selection: the left screen shows a row per variable with its NOUN, label
 *    and current value, the highlighted row marked by a cursor block (like
 *    VERB 8); the user picks the target either by typing its NOUN number and
 *    pressing ENTER, or by navigating with +/- and pressing ENTER. The NOUN
 *    label is on but the NOUN field stays blank until a number is typed.
 *  - Value entry: the left screen keeps showing the rows with the selected
 *    row marked; the right screen shows the selected variable's NOUN, an
 *    "in" prompt in R1 and the value being typed in R2 (blinking label);
 *    ENTER commits the write and finishes.
 *
 * Key handling:
 *  - selection phase: digits build the pending NOUN number (five-digit cap);
 *    +/- cycle the highlighted variable with wrap-around (like VERB 10);
 *    ENTER selects the typed NOUN if it matches a variable (otherwise the
 *    key is consumed and the number stays on screen) or commits the current
 *    highlight; CLR cancels.
 *  - value entry phase: digits build the value (six-digit cap); ENTER
 *    commits it to the selected NOUN and finishes (a rejected write, e.g. a
 *    non-0/1 for a bool variable, stays in value entry); CLR cancels without
 *    writing and finishes.
 *  - any other key is ignored (RSET exits to the VERB 0 menu).
 */
class WriteVar : public Action {
 public:

  /**
   * @brief Construct the WRITE_VAR action.
   *
   * @param store Variable storage backend shared with all variable actions.
   * @param hmi HMI data object used to draw the LCD screens.
   */
  WriteVar(VariableStore &store, HmiData &hmi);

  ~WriteVar() override = default;

  void operation() override;

  bool handleKey(EventId_t key) override;

 protected:

  /**
   * @brief Initialise per-run state at the start of every run.
   *
   * Resets to the selection phase: no NOUN number has been typed yet and no
   * value is pending (both blanked by updateScreens()); the highlight starts
   * at the top of the variable list.
   */
  void start() override;

 private:

  /**
   * @brief Redraw both LCD screens from the current state.
   *
   * Called by start() on entry and by handleKey() after any accepted key, so
   * the display reacts to keypresses immediately instead of waiting for the
   * next tick. operation() does not call it (no periodic redraw).
   */
  void updateScreens();

  VariableStore &store_;      //!< Variable storage backend

  std::atomic<int> phase_{0};           //!< Current phase: 0 = selection, 1 = value entry
  std::atomic<int> pendingNoun_ {-1};   //!< NOUN number being typed in the selection phase (-1 = none)
  std::atomic<int> pendingValue_ {-1};  //!< Value being typed / awaiting commit (-1 = none)
  std::atomic<int> selected_{-1};       //!< Index of the current write target (-1 = none)
};
