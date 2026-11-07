#include "WriteVar.h"

#include "VariableStore.h"

#include "ListOfActions.h"

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
 * sees matches the NOUN that will be written even if NOUNs have gaps. Rows
 * follow NOUN order (the VariableStore map is keyed by NOUN ascending). Only
 * glyphs the Gorton font can draw are used; values are non-negative integers.
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

/**
 * @brief Map a digit key event to its numeric value.
 *
 * Returns 0-9 for E_KEY_0..E_KEY_9 and -1 for any other key id, so callers
 * can test "is this a digit?" with a single comparison. Mirrors the style of
 * HmiLogic's file-local key() helper.
 */
int digitOf(EventId_t id)
{
  int value = -1;

  switch (id) {
  case E_KEY_0:
    value = 0;
    break;

  case E_KEY_1:
    value = 1;
    break;

  case E_KEY_2:
    value = 2;
    break;

  case E_KEY_3:
    value = 3;
    break;

  case E_KEY_4:
    value = 4;
    break;

  case E_KEY_5:
    value = 5;
    break;

  case E_KEY_6:
    value = 6;
    break;

  case E_KEY_7:
    value = 7;
    break;

  case E_KEY_8:
    value = 8;
    break;

  case E_KEY_9:
    value = 9;
    break;

  default:
    break;
  }

  return value;
}

} // namespace

/**
 * @brief Construct the WRITE_VAR action.
 *
 * The action runs periodically so it stays alive while the user browses, but
 * every screen update is event-driven: start() draws once on entry and
 * handleKey redraws on +/- presses. operation() therefore does nothing; a
 * periodic full reset+redraw here would flicker at this tick rate.
 */
WriteVar::WriteVar(VariableStore &store, HmiData &hmi)
    : Action(WRITE_VAR_VERB, WRITE_VAR_NOUN, hmi, ACTION_PERIODIC, "WRITE VAR"), store_(store)
{
  setPeriod(100);
}

void WriteVar::start()
{
  // Verb-only entry: no variable has been chosen yet, so start in the
  // selection phase. No NOUN has been typed and no value is pending; both
  // fields are blanked by updateScreens().
  phase_.store(0);
  pendingNoun_.store(-1);
  pendingValue_.store(-1);

  // No specific variable was requested, so start at the top of the list;
  // the user navigates with +/- or types a NOUN number.
  selected_.store(0);
  updateScreens();
}

bool WriteVar::handleKey(EventId_t key)
{
  const std::vector<VariableStore::VarInfo> vars = store_.varList();
  const int count = static_cast<int>(vars.size());

  // Selection phase: pick the target variable by typing its NOUN number or
  // with +/-; ENTER moves to value entry.
  if (phase_.load() == 0) {
    const int d = digitOf(key);
    if (d >= 0) {
      int v = pendingNoun_.load();
      if (v < 0) {
        // First digit starts the NOUN number.
        v = d;
      } else if (v < 10000) {
        // Shift left and append; capped at five digits (max 99999).
        v = v * 10 + d;
      }
      pendingNoun_.store(v);
      updateScreens();
      return true;
    }

    // +/- move the highlight with wrap-around, like VERB 10.
    const int cur = selected_.load();
    if (key == E_KEY_PLUS) {
      if (count == 0) {
        return false;
      }
      selected_.store(cur < 0 ? 0 : (cur + 1) % count);
      updateScreens();
      return true;
    }
    if (key == E_KEY_MINUS) {
      if (count == 0) {
        return false;
      }
      selected_.store(cur < 0 ? count - 1 : (cur - 1 + count) % count);
      updateScreens();
      return true;
    }

    // ENTER: a typed NOUN number must match a variable to be accepted; when
    // no number is typed the current highlight is committed. Both cases move
    // to the value entry phase. A non-matching number is consumed but kept
    // on screen so the user can see what was typed.
    if (key == E_KEY_ENTR) {
      const int pn = pendingNoun_.load();
      if (pn >= 0) {
        for (int i = 0; i < count; ++i) {
          if (vars[i].noun == pn) {
            selected_.store(i);
            pendingNoun_.store(-1);
            phase_.store(1);
            updateScreens();
            return true;
          }
        }
        // No variable has this NOUN: keep the typed number, stay put.
        return true;
      }
      if (cur >= 0 && cur < count) {
        phase_.store(1);
        updateScreens();
      }
      return true;
    }

    // CLR cancels the whole write and finishes.
    if (key == E_KEY_CLR) {
      hmiData_.resetLeftLcdData();
      finish();
      return true;
    }

    // Any other key is ignored.
    return false;
  }

  // Value entry phase: digits build the value to write.
  const int d = digitOf(key);
  if (d >= 0) {
    int v = pendingValue_.load();
    if (v < 0) {
      // First digit starts the value.
      v = d;
    } else if (v < 100000) {
      // Shift left and append; capped at six digits (max 999999).
      v = v * 10 + d;
    }
    pendingValue_.store(v);
    updateScreens();
    return true;
  }

  // ENTER commits the value to the selected NOUN and finishes. With no
  // value typed yet it is consumed but nothing changes. If set() rejects
  // the value (e.g. a non-0/1 for a bool variable), stay in this phase so
  // the user can retype or cancel with CLR. The left list is reset on
  // success because the HMI reap path only resets the right LCD.
  if (key == E_KEY_ENTR) {
    const int v = pendingValue_.load();
    if (v < 0) {
      return true;
    }
    const int cur = selected_.load();
    if (cur >= 0 && cur < count && !store_.set(vars[cur].noun, v)) {
      return true;
    }
    hmiData_.resetLeftLcdData();
    finish();
    return true;
  }

  // CLR cancels without writing and finishes.
  if (key == E_KEY_CLR) {
    hmiData_.resetLeftLcdData();
    finish();
    return true;
  }

  // All other keys are ignored; the action also ends on RSET, which the HMI
  // layer handles outside this action.
  return false;
}

void WriteVar::operation()
{
  // No periodic redraw: screens are updated on entry (start) and on +/- key
  // presses (handleKey). Redrawing every tick would clear+repaint the LCD and
  // flicker at this period. The tick only keeps the action alive so it keeps
  // receiving keys until ENTER/CLR/RSET ends it.
}

void WriteVar::updateScreens()
{
  const std::vector<VariableStore::VarInfo> vars = store_.varList();
  const int sel = selected_.load();

  // Left screen: the variable rows (NOUN, label and live value) with the
  // current target marked by a cursor block, shown in both phases so the user
  // always sees which variable is active.
  hmiData_.resetLeftLcdData();
  hmiData_.showVars(buildVarLines(store_, vars), sel);

  // Right screen: the NOUN label stays on in both phases; the NOUN field shows
  // the number being typed (phase 0) or the selected variable's NOUN (phase 1).
  hmiData_.rSetCompActLabel(DRAW_ON);
  hmiData_.rSetNounLabel(DRAW_ON);
  hmiData_.rSetnNoun(0, DRAW_OFF);

  if (phase_.load() == 0) {
    // Selection phase: NOUN field shows the number being typed, if any.
    const int pn = pendingNoun_.load();
    if (pn >= 0) {
      hmiData_.rSetnNoun(pn, DRAW_ON);
    }

    // R1 and R2 blanked: no prompt or value yet.
    hmiData_.rSetR1Label(DRAW_OFF);
    hmiData_.rSetnR1(0, DRAW_OFF);
    hmiData_.rSetR2Label(DRAW_OFF);
    hmiData_.rSetnR2(UNASIGNED, DRAW_OFF);
  } else {
    // Value entry phase: NOUN field shows the selected variable's NOUN and R1
    // prompts "in" (same sequence as the DBMONITOR period prompt in HmiLogic).
    if (sel >= 0 && sel < static_cast<int>(vars.size())) {
      hmiData_.rSetnNoun(vars[sel].noun, DRAW_ON);
    }

    hmiData_.rSetR2Label(DRAW_BLINK);
    hmiData_.rSetR1Label(DRAW_ON);
    hmiData_.rSetStrR1("in", DRAW_ON);

    // R2: the value being typed, blank until the first digit.
    const int v = pendingValue_.load();
    if (v < 0) {
      hmiData_.rSetnR2(UNASIGNED, DRAW_OFF);
    } else {
      hmiData_.rSetnR2(v, DRAW_ON);
    }
  }

  // R3 blanked.
  hmiData_.rSetR3Label(DRAW_OFF);
  hmiData_.rSetnR3(0, DRAW_OFF);
}
