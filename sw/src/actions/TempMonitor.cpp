#include "TempMonitor.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

/**
 * @brief Dummy temperature value (deg C) shown until a real sensor is wired up.
 */
static const int DUMMY_TEMP_C = 25;

/**
 * @brief Constructor for the TEMP_MONITOR action.
 *
 * Registers as a periodic action: operation() runs every second to refresh
 * the temperature display (currently a constant dummy value).
 */
TempMonitor::TempMonitor(HmiData &data) : Action(TEMP_MONITOR_VERB, TEMP_MONITOR_NOUN, data, ACTION_PERIODIC, "TEMP MONITOR")
{
  setPeriod(1000);
}

/**
 * @brief Clear the left screen, enable the right-screen "COMP ACT" label and
 *        draw the (dummy) temperature in R1.
 */
void TempMonitor::operation()
{
  hmiData_.resetLeftLcdData();
  hmiData_.rSetCompActLabel(DRAW_ON);

  hmiData_.rSetnR1(DUMMY_TEMP_C, DRAW_ON);
  hmiData_.rSetR1Label(DRAW_ON);
}
