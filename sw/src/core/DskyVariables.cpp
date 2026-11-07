#include "DskyVariables.h"

// Default constants for the screensaver variables
#include "ScreenSaver.h"

// Default constant for the DBMONITOR period variable
#include "DbMonitorProgram.h"

DskyVariables::DskyVariables(VariableStore &store)
{
    // NOUN 2: keypad light on/off. Apply callback attached by the caller.
    store.registerVar(NOUN_KEYPAD_LIGHT, "keypadLight", "KEYPAD LIGHT", VAR_BOOL, 0);

    // NOUN 3: screensaver enabled/disabled. Apply callback attached by the caller.
    store.registerVar(NOUN_SCREENSAVER, "screensaver", "SCREENSAVER", VAR_BOOL, SCREENSAVER_DEFAULT);

    // NOUN 4: screensaver idle timer in seconds. Apply callback attached by the caller.
    store.registerVar(NOUN_SCREENSAVER_TIMER, "screensaverTimer", "SCREENSAVER TIMER", VAR_INT, SCREENSAVER_TIMER);

    // NOUN 5: DBMONITOR execution period in seconds. The program reads it at start().
    store.registerVar(NOUN_DBMONITOR_PERIOD, "dbMonitorPeriod", "DBMONITOR PERIOD", VAR_INT, DBMONITOR_DEFAULT_PERIOD);

    // NOUN 6: DummyProgram auto-start at boot. Boot-only: main.cpp reads it once
    // after registration; changing it later takes effect on the next boot. No apply callback.
    store.registerVar(NOUN_DUMMY_AUTOSTART, "dummyAutostart", "DUMMY AUTOSTART", VAR_BOOL, 0);

    // NOUN 7: DbMonitorProgram auto-start at boot. Boot-only, same semantics as NOUN 6.
    store.registerVar(NOUN_DBMONITOR_AUTOSTART, "dbMonitorAutostart", "DBMONITOR AUTOSTART", VAR_BOOL, 0);

    // NOUN 8: UdpDoorProgram auto-start at boot. Boot-only, same semantics as NOUN 6.
    // Defaults to 1 (on): the door monitor is a passive external-signal catcher
    // and should be live right after deployment without keypad interaction.
    store.registerVar(NOUN_UDPDOOR_AUTOSTART, "udpdoorAutostart", "UDPDOOR AUTOSTART", VAR_BOOL, 1);
}
