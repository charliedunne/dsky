#include "ReadVar.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

/**
 * @brief Build the left-screen variable rows.
 *
 * Each row has the form "<NOUN> <LABEL> <VALUE>" showing the variable's NOUN
 * id, label and current value, e.g. "2 KEYPAD LIGHT 1". The leading number is
 * the variable's actual NOUN id (not a 1-based position), so what the user
 * sees matches the NOUN that will be read even if NOUNs have gaps. Rows follow
 * NOUN order (the VariableStore map is keyed by NOUN ascending). Only glyphs
 * the Gorton font can draw are used; values are non-negative integers.
 */
std::vector<std::string> buildVarLines(const VariableStore &store, const std::vector<VariableStore::VarInfo> &vars)
{
  std::vector<std::string> lines;
  lines.reserve(vars.size());
  char line[40];
  for (size_t i = 0; i < vars.size(); ++i) {
    int value = 0;
    if (store.get(vars[i].noun, value)) {
      snprintf(line, sizeof(line), "%d %s %d", vars[i].noun, vars[i].label.c_str(), value);
    } else {
      snprintf(line, sizeof(line), "%d %s", vars[i].noun, vars[i].label.c_str());
    }
    lines.emplace_back(line);
  }
  return lines;
}

} // namespace

/**
 * @brief Construct the READ_VAR action.
 *
 * The action runs periodically so it stays alive while the user browses, but
 * every screen update is event-driven: start() draws once on entry and
 * handleKey redraws on +/- presses. operation() therefore does nothing; a
 * periodic full reset+redraw here would flicker at this tick rate.
 */
ReadVar::ReadVar(VariableStore &store, HmiData &hmi)
    : Action(READ_VAR_VERB, READ_VAR_NOUN, hmi, ACTION_PERIODIC, "READ VAR"), store_(store)
{
  setPeriod(100);
}

void ReadVar::start()
{
  // Verb-only entry: no specific variable was requested, so start at the
  // top of the list. The user navigates with +/-.
  selected_.store(0);
  updateScreens();
}

bool ReadVar::handleKey(EventId_t key)
{
  const std::vector<VariableStore::VarInfo> vars = store_.varList();
  const int count = static_cast<int>(vars.size());
  if (count == 0) {
    return false;
  }

  const int cur = selected_.load();

  // +/- move the selection with wrap-around, like VERB 10.
  if (key == E_KEY_PLUS) {
    selected_.store(cur < 0 ? 0 : (cur + 1) % count);
    updateScreens();
    return true;
  }
  if (key == E_KEY_MINUS) {
    selected_.store(cur < 0 ? count - 1 : (cur - 1 + count) % count);
    updateScreens();
    return true;
  }

  /* ENTER exits the list (symmetric with WriteVar). RSET is handled by the
   * HMI layer and also terminates the action */
  if (key == E_KEY_ENTR)
  {
    hmiData_.resetLeftLcdData();
    finish();
    return true;
  }

  return false;
}

void ReadVar::operation()
{
  // No periodic redraw: screens are updated on entry (start) and on +/- key
  // presses (handleKey). Redrawing every tick would clear+repaint the LCD and
  // flicker at this period. The tick only keeps the action alive so it keeps
  // receiving keys until RSET/ENTER ends it.
}

void ReadVar::updateScreens()
{
  const std::vector<VariableStore::VarInfo> vars = store_.varList();
  const int sel = selected_.load();

  // Left screen: variable rows (NOUN, label and live value) with the selected
  // row marked by a cursor block.
  hmiData_.resetLeftLcdData();
  hmiData_.showVars(buildVarLines(store_, vars), sel);

  // Right screen: NOUN field shows the selected variable's NOUN and R2 its
  // current value; R1 and R3 are blanked.
  hmiData_.rSetCompActLabel(DRAW_ON);
  hmiData_.rSetNounLabel(DRAW_ON);
  hmiData_.rSetnNoun(0, DRAW_OFF);

  int value = 0;
  bool haveValue = false;
  if (sel >= 0 && sel < static_cast<int>(vars.size())) {
    const VariableStore::VarInfo &v = vars[sel];
    hmiData_.rSetnNoun(v.noun, DRAW_ON);
    haveValue = store_.get(v.noun, value);
  }

  // R1 blanked.
  hmiData_.rSetR1Label(DRAW_OFF);
  hmiData_.rSetnR1(0, DRAW_OFF);

  // R2: current value of the selected variable.
  hmiData_.rSetR2Label(haveValue ? DRAW_ON : DRAW_OFF);
  hmiData_.rSetnR2(value, haveValue ? DRAW_ON : DRAW_OFF);

  // R3 blanked.
  hmiData_.rSetR3Label(DRAW_OFF);
  hmiData_.rSetnR3(0, DRAW_OFF);
}
