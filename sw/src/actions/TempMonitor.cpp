#include "TempMonitor.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

#include <array>
#include <cstdio>
#include <string>
#include <vector>

namespace {

/**
 * @brief A dummy sensor entry in the device list.
 */
struct Sensor {
  int id;         //!< NOUN ID (1..TEMP_MONITOR_NOUN_MAX + 1)
  const char *name;  //!< Display name (up to 7 chars)
  float tempC;    //!< Constant temperature value (deg C)
};

/**
 * @brief Table of dummy sensors shown during VERB 10.
 */
constexpr std::array<Sensor, TEMP_MONITOR_NOUN_MAX + 1> SENSORS = {{
    {1, "KITCHEN", 19.40f},
    {2, "LIVING", 22.50f},
    {3, "BEDROOM", 21.75f},
    {4, "GARAGE", 12.30f},
    {5, "OUTSIDE", -5.20f},
}};

/**
 * @brief Build the left-screen device list lines.
 *
 * Each line has the form "<id> <NAME>". The selected line's name is wrapped
 * in asterisks, e.g. "2 *LIVING*"; unselected lines are plain, e.g. "1 KITCHEN".
 */
std::vector<std::string> buildListLines(int selected)
{
  std::vector<std::string> lines;
  lines.reserve(SENSORS.size());
  char line[28];
  for (const auto &s : SENSORS) {
    const bool sel = (s.id - 1) == selected;
    snprintf(line, sizeof(line), "%d %s%s%s", s.id, sel ? "*" : "", s.name, sel ? "*" : "");
    lines.emplace_back(line);
  }
  return lines;
}

} // namespace

/**
 * @brief Constructor for the TEMP_MONITOR action.
 *
 * Registers as a periodic action: operation() runs every second to refresh
 * the device list and the selected sensor's temperature.
 */
TempMonitor::TempMonitor(HmiData &data) : Action(TEMP_MONITOR_VERB, TEMP_MONITOR_NOUN, data, ACTION_PERIODIC, "TEMP MONITOR")
{
  setPeriod(1000);
}

void TempMonitor::start()
{
  /* Reset per-run state so every run starts with nothing selected */
  selected_ = -1;
  entering_ = false;
  entryValue_ = 0;
  entryDigits_ = 0;
}

bool TempMonitor::enterDigit(int d)
{
  if (entryDigits_.load() >= 2) {
    return false;
  }
  entryValue_.store(entryValue_.load() * 10 + d);
  entryDigits_.fetch_add(1);
  return true;
}

bool TempMonitor::handleKey(EventId_t key)
{
  // Map the key to a digit; -1 if it is not a digit. The digit codes are
  // non-contiguous bit patterns, so a switch is required.
  int d = -1;
  switch (key) {
    case E_KEY_0: d = 0; break;
    case E_KEY_1: d = 1; break;
    case E_KEY_2: d = 2; break;
    case E_KEY_3: d = 3; break;
    case E_KEY_4: d = 4; break;
    case E_KEY_5: d = 5; break;
    case E_KEY_6: d = 6; break;
    case E_KEY_7: d = 7; break;
    case E_KEY_8: d = 8; break;
    case E_KEY_9: d = 9; break;
    default: break;
  }

  if (entering_.load()) {
    if (d >= 0) {
      const bool ok = enterDigit(d);
      // Redraw immediately so the digit appears without waiting for the tick.
      if (ok) {
        updateScreens();
      }
      return ok;
    }
    if (key == E_KEY_ENTR) {
      // ENTER with no digits typed: do nothing, stay in entry mode.
      if (entryDigits_.load() == 0) {
        return true;
      }
      const int v = entryValue_.load();
      if (v >= 1 && v <= TEMP_MONITOR_NOUN_MAX + 1) {
        selected_.store(v - 1);
        entering_.store(false);
        entryValue_.store(0);
        entryDigits_.store(0);
        updateScreens();
        return true;
      }
      // Out of range: stay in entry mode, keeping the pending value.
      return false;
    }
    if (key == E_KEY_CLR) {
      // Cancel the entry, keeping the current selection.
      entering_.store(false);
      entryValue_.store(0);
      entryDigits_.store(0);
      updateScreens();
      return true;
    }
    return false;
  }

  // Normal mode: cycle the selection or start a NOUN entry.
  const int count = TEMP_MONITOR_NOUN_MAX + 1;
  if (key == E_KEY_PLUS) {
    // First press selects device 1; afterwards cycle with wrap-around.
    const int cur = selected_.load();
    selected_.store(cur < 0 ? 0 : (cur + 1) % count);
    updateScreens();
    return true;
  }
  if (key == E_KEY_MINUS) {
    // First press selects the last device; afterwards cycle with wrap-around.
    const int cur = selected_.load();
    selected_.store(cur < 0 ? count - 1 : (cur - 1 + count) % count);
    updateScreens();
    return true;
  }
  if (key == E_KEY_CLR || key == E_KEY_NOUN) {
    entering_.store(true);
    entryValue_.store(0);
    entryDigits_.store(0);
    updateScreens();
    return true;
  }
  return false;
}

void TempMonitor::operation()
{
  updateScreens();
}

void TempMonitor::updateScreens()
{
  const int sel = selected_.load();

  // Left screen: device list with the selected line marked.
  hmiData_.resetLeftLcdData();
  hmiData_.lSetVerbListHeader(0, "DEVICES");
  hmiData_.lSetVerbList(buildListLines(sel), {});

  // Right screen: three display states.
  //  - Normal (a sensor is selected): NOUN field shows the selected ID, and
  //    the temperature is split across R1 (integer part, with sign), R2
  //    (fraction digits, no dot) and R3 (unit "ºC").
  //  - Entry mode: NOUN label lit to indicate a device ID is expected; once
  //    digits are typed the pending value is shown live in the NOUN field.
  //    R1/R2/R3 stay off until ENTER confirms the selection.
  //  - Otherwise (nothing selected yet): all fields blank.
  hmiData_.rSetCompActLabel(DRAW_ON);
  if (!entering_.load() && sel >= 0) {
    hmiData_.rSetNounLabel(DRAW_ON);
    hmiData_.rSetnNoun(sel + 1, DRAW_ON);
    hmiData_.rSetR1Label(DRAW_ON);
    hmiData_.rSetR2Label(DRAW_ON);
    hmiData_.rSetR3Label(DRAW_ON);
    // Split the temperature across three rows: R1 = integer part (with sign),
    // R2 = fraction digits only (no dot, per user preference), R3 = unit.
    // %.2f guarantees a dot is present.
    char buf[16];
    snprintf(buf, sizeof(buf), "%+.2f", SENSORS[sel].tempC);
    std::string full(buf);
    const size_t dot = full.find('.');
    // The Sign widget renders the sign itself; strip a leading '+' from R1.
    std::string r1 = full.substr(0, dot);
    if (!r1.empty() && r1[0] == '+') {
      r1.erase(0, 1);
    }
    // Fraction without the dot (e.g. "50"); the Dot glyph is not used here.
    const std::string r2 = full.substr(dot + 1);
    hmiData_.rSetStrR1(r1, DRAW_ON, /*showSign=*/true);
    hmiData_.rSetStrR2(r2, DRAW_ON);
    // No space between º and C: ' ' is not a mapped Digit glyph and would
    // render as garbage. UTF-8 "º" -> single sentinel byte.
    // The two literals are concatenated on purpose: in "\xBAC" the hex
    // escape greedily consumes the trailing 'C' (a hex digit) and yields
    // 0xAC instead of 0xBA followed by 'C'.
    hmiData_.rSetStrR3("\xC2\xBA" "C", DRAW_ON);
  } else if (entering_.load()) {
    // Entry mode: NOUN label always lit as a prompt for the device ID.
    hmiData_.rSetNounLabel(DRAW_ON);
    if (entryDigits_.load() > 0) {
      // Digits typed: pending value shown live in the NOUN field.
      hmiData_.rSetnNoun(entryValue_.load(), DRAW_ON);
    } else {
      hmiData_.rSetnNoun(0, DRAW_OFF);
    }
    hmiData_.rSetR1Label(DRAW_OFF);
    hmiData_.rSetnR1(0, DRAW_OFF);
    hmiData_.rSetR2Label(DRAW_OFF);
    hmiData_.rSetnR2(0, DRAW_OFF);
    hmiData_.rSetR3Label(DRAW_OFF);
    hmiData_.rSetnR3(0, DRAW_OFF);
  } else {
    // Nothing selected yet, or waiting for the first digit: all blank.
    hmiData_.rSetNounLabel(DRAW_OFF);
    hmiData_.rSetnNoun(0, DRAW_OFF);
    hmiData_.rSetR1Label(DRAW_OFF);
    hmiData_.rSetnR1(0, DRAW_OFF);
    hmiData_.rSetR2Label(DRAW_OFF);
    hmiData_.rSetnR2(0, DRAW_OFF);
    hmiData_.rSetR3Label(DRAW_OFF);
    hmiData_.rSetnR3(0, DRAW_OFF);
  }
}
