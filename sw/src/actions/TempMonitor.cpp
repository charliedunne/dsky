#include "TempMonitor.h"

// Logging
#include "ListOfActions.h"
#include "Logger.h"

/**
 * @brief Constructor for the TEMP_MONITOR action (step 1: screen setup only).
 *
 * Registers as a single-shot action; will become ACTION_PERIODIC once real
 * temperature monitoring is implemented in a later step.
 */
TempMonitor::TempMonitor(HmiData &data) : Action(TEMP_MONITOR_VERB, TEMP_MONITOR_NOUN, data, ACTION_SINGLE, "TEMP MONITOR")
{
}

/**
 * @brief Step 1 behavior: clear the left screen and enable the right-screen
 *        "COMP ACT" label. Real temperature monitoring comes in later steps.
 */
void TempMonitor::operation()
{
  hmiData_.resetLeftLcdData();
  hmiData_.rSetCompActLabel(DRAW_ON);
}
