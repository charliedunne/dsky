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
  int tempC;      //!< Constant temperature value (deg C)
};

/**
 * @brief Table of dummy sensors shown during VERB 10.
 */
constexpr std::array<Sensor, TEMP_MONITOR_NOUN_MAX + 1> SENSORS = {{
    {1, "KITCHEN", 19},
    {2, "LIVING", 22},
    {3, "BEDROOM", 21},
    {4, "GARAGE", 12},
    {5, "OUTSIDE", -5},
}};

/**
 * @brief Build the left-screen device list lines.
 *
 * Each line has the form "<marker> <id> <NAME> <temp>C" where the marker is
 * '>' on the selected line and a space otherwise.
 */
std::vector<std::string> buildListLines(int selected)
{
  std::vector<std::string> lines;
  lines.reserve(SENSORS.size());
  char line[28];
  for (const auto &s : SENSORS) {
    snprintf(line, sizeof(line), "%c %d %-7s %dC", (s.id - 1) == selected ? '>' : ' ', s.id, s.name, s.tempC);
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
      return enterDigit(d);
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
      return true;
    }
    return false;
  }

  // Normal mode: cycle the selection or start a NOUN entry.
  const int count = TEMP_MONITOR_NOUN_MAX + 1;
  if (key == E_KEY_PLUS) {
    selected_.store((selected_.load() + 1) % count);
    return true;
  }
  if (key == E_KEY_MINUS) {
    selected_.store((selected_.load() - 1 + count) % count);
    return true;
  }
  if (key == E_KEY_CLR || key == E_KEY_NOUN) {
    entering_.store(true);
    entryValue_.store(0);
    entryDigits_.store(0);
    return true;
  }
  return false;
}

void TempMonitor::operation()
{
  const int sel = selected_.load();

  // Left screen: device list with the selected line marked.
  hmiData_.resetLeftLcdData();
  hmiData_.lSetVerbListHeader(0, "DEVICES");
  hmiData_.lSetVerbList(buildListLines(sel), {});

  // Right screen: three display states.
  //  - Normal: NOUN field shows the selected ID, R1 shows its temperature.
  //  - Entry, no digits yet: all four fields blank.
  //  - Entry, digit(s) typed: NOUN shows the pending value live; R1 stays off
  //    until ENTER confirms the selection.
  hmiData_.rSetCompActLabel(DRAW_ON);
  if (!entering_.load()) {
    hmiData_.rSetNounLabel(DRAW_ON);
    hmiData_.rSetnNoun(sel + 1, DRAW_ON);
    hmiData_.rSetR1Label(DRAW_ON);
    // String form so we can append the "ºC" suffix (max 5 chars).
    // showSign=true lets the Sign widget render a '-' for negative temps.
    hmiData_.rSetStrR1(std::to_string(SENSORS[sel].tempC) + "ºC", DRAW_ON, /*showSign=*/true);
  } else if (entryDigits_.load() == 0) {
    // Waiting for the first digit: everything blank.
    hmiData_.rSetNounLabel(DRAW_OFF);
    hmiData_.rSetnNoun(0, DRAW_OFF);
    hmiData_.rSetR1Label(DRAW_OFF);
    hmiData_.rSetnR1(0, DRAW_OFF);
  } else {
    // Digits typed: pending value shown live in the NOUN field.
    hmiData_.rSetNounLabel(DRAW_ON);
    hmiData_.rSetnNoun(entryValue_.load(), DRAW_ON);
    hmiData_.rSetR1Label(DRAW_OFF);
    hmiData_.rSetnR1(0, DRAW_OFF);
  }
}
