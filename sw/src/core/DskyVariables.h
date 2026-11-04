#ifndef _DSKYVARIABLES_H_
#define _DSKYVARIABLES_H_

#include "VariableStore.h"

/**
 * @brief Central registry of all DSKY persistent variables.
 *
 * Constructing a DskyVariables instance registers every variable known to the
 * system into the given VariableStore, with its name, label, type and default
 * value. No hardware apply callbacks are attached here: they are attached by
 * the caller (main) through VariableStore::setApply() after construction, so
 * that this class does not need to know about the hardware objects it must
 * drive.
 */
class DskyVariables
{
public:
    /** NOUN 2: keypad light on/off (bool). */
    static constexpr int NOUN_KEYPAD_LIGHT = 2;

    /** NOUN 3: screensaver enabled/disabled (bool). */
    static constexpr int NOUN_SCREENSAVER = 3;

    /** NOUN 4: screensaver idle timer in seconds (int). */
    static constexpr int NOUN_SCREENSAVER_TIMER = 4;

    /** NOUN 5: DBMONITOR execution period in seconds (int). */
    static constexpr int NOUN_DBMONITOR_PERIOD = 5;

    /**
     * @brief Register all DSKY variables into the store.
     * @param store Store that will own the variable definitions
     */
    explicit DskyVariables(VariableStore &store);
};

#endif /* _DSKYVARIABLES_H_ */
