#include "TempMonitor.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"
#include "DbClient.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

namespace {

/**
 * @brief Age (seconds) after which a sensor reading is shown as STALE.
 */
constexpr int TEMP_STALE_THRESHOLD_S = 600;

/**
 * @brief Strip a trailing "_TEMP" suffix from a device name for display.
 *
 * All DS18B20 short_names end in "_TEMP"; dropping it keeps list lines
 * compact (e.g. "TRAINING_ROOM_TEMP" -> "TRAINING_ROOM"). Names without the
 * suffix are returned unchanged so the helper stays safe for any device type.
 */
std::string displayName(const std::string &name)
{
  static const std::string kSuffix = "_TEMP";
  if (name.size() > kSuffix.size() &&
      name.compare(name.size() - kSuffix.size(), kSuffix.size(), kSuffix) == 0) {
    return name.substr(0, name.size() - kSuffix.size());
  }
  return name;
}

/**
 * @brief Build the left-screen device list lines.
 *
 * Each line has the form "<position> <NAME>". The selected line's name is
 * wrapped in asterisks, e.g. "2 *LIVING*"; unselected lines are plain, e.g.
 * "1 KITCHEN". Positions are 1-based and follow the sensor order returned by
 * the database, so ENTER (which maps position - 1 to a list index) always
 * matches the displayed number even if device IDs have gaps.
 */
std::vector<std::string> buildListLines(const std::vector<DbReading> &readings, int selected)
{
  const size_t n = std::min(readings.size(), static_cast<size_t>(TEMP_MONITOR_NOUN_MAX + 1));
  std::vector<std::string> lines;
  lines.reserve(n);
  char line[28];
  for (size_t i = 0; i < n; ++i) {
    const bool sel = static_cast<int>(i) == selected;
    const std::string name = displayName(readings[i].name);
    snprintf(line, sizeof(line), "%d %s%s%s", static_cast<int>(i + 1), sel ? "*" : "", name.c_str(), sel ? "*" : "");
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
TempMonitor::TempMonitor(HmiData &data, DbClient &db) : Action(TEMP_MONITOR_VERB, TEMP_MONITOR_NOUN, data, ACTION_PERIODIC, "TEMP MONITOR"), db_(db)
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

  // Number of sensors actually available right now (may be less than the
  // maximum while the database has no data yet).
  const int count = std::min(TEMP_MONITOR_NOUN_MAX + 1, static_cast<int>(db_.snapshot().size()));

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
      if (v >= 1 && v <= count) {
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
  if (key == E_KEY_PLUS) {
    if (count == 0) {
      return false;
    }
    // First press selects device 1; afterwards cycle with wrap-around.
    const int cur = selected_.load();
    selected_.store(cur < 0 ? 0 : (cur + 1) % count);
    updateScreens();
    return true;
  }
  if (key == E_KEY_MINUS) {
    if (count == 0) {
      return false;
    }
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

  // Current sensor readings (cheap copy under a brief lock) and the number of
  // sensors available right now.
  const std::vector<DbReading> readings = db_.snapshot();
  const int count = std::min(TEMP_MONITOR_NOUN_MAX + 1, static_cast<int>(readings.size()));

  // Left screen: device list with the selected line marked.
  hmiData_.resetLeftLcdData();
  hmiData_.lSetVerbListHeader(0, "DEVICES");
  hmiData_.lSetVerbList(buildListLines(readings, sel), {});

  // Right screen: three display states.
  //  - Normal (a sensor is selected): NOUN field shows the selected position,
  //    and the temperature is split across R1 (integer part, with sign), R2
  //    (fraction digits, no dot) and R3 (unit "ºC", or "STALE" when the last
  //    reading is older than TEMP_STALE_THRESHOLD_S). A sensor that has never
  //    delivered a valid reading keeps its labels lit but shows blank values.
  //  - Entry mode: NOUN label lit to indicate a device ID is expected; once
  //    digits are typed the pending value is shown live in the NOUN field.
  //    R1/R2/R3 stay off until ENTER confirms the selection.
  //  - Otherwise (nothing selected yet): all fields blank.
  hmiData_.rSetCompActLabel(DRAW_ON);
  if (!entering_.load() && sel >= 0 && sel < count) {
    const DbReading &r = readings[sel];
    hmiData_.rSetNounLabel(DRAW_ON);
    hmiData_.rSetnNoun(sel + 1, DRAW_ON);
    hmiData_.rSetR1Label(DRAW_ON);
    hmiData_.rSetR2Label(DRAW_ON);
    hmiData_.rSetR3Label(DRAW_ON);
    if (!r.valid) {
      // No reading has ever been fetched for this sensor: keep the labels lit
      // but leave the value fields blank.
      hmiData_.rSetStrR1("", DRAW_OFF);
      hmiData_.rSetStrR2("", DRAW_OFF);
      hmiData_.rSetStrR3("", DRAW_OFF);
    } else {
      const bool stale = (time(nullptr) - r.lastUpdate) > TEMP_STALE_THRESHOLD_S;
      // Split the temperature across three rows: R1 = integer part (with sign),
      // R2 = fraction digits only (no dot, per user preference), R3 = unit.
      // %.2f guarantees a dot is present.
      char buf[16];
      snprintf(buf, sizeof(buf), "%+.2f", r.tempC);
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
      if (stale) {
        // Keep the last known value but flag it as stale.
        hmiData_.rSetStrR3("STALE", DRAW_ON);
      } else {
        // No space between º and C: ' ' is not a mapped Digit glyph and would
        // render as garbage. UTF-8 "º" -> single sentinel byte.
        // The two literals are concatenated on purpose: in "\xBAC" the hex
        // escape greedily consumes the trailing 'C' (a hex digit) and yields
        // 0xAC instead of 0xBA followed by 'C'.
        hmiData_.rSetStrR3("\xC2\xBA" "C", DRAW_ON);
      }
    }
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
