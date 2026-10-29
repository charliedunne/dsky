#include "DisableScreenSaver.h"

// Configuration
#include "ListOfActions.h"

// Logging
#include "Logger.h"

DisableScreenSaver::DisableScreenSaver(HmiData &data, ScreenSaver &saver) : Action(DISABLE_SCREENSAVER_VERB, DISABLE_SCREENSAVER_NOUN, data, ACTION_SINGLE, "SCREENSAVER OFF"), saver_(saver)
{
}

void DisableScreenSaver::operation()
{
    LogTrace << "Running ScreenSaver OFF" << std::endl;

    saver_.setEnabled(false);
}
