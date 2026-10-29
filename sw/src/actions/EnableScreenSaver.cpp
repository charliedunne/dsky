#include "EnableScreenSaver.h"

// Configuration
#include "ListOfActions.h"

// Logging
#include "Logger.h"

EnableScreenSaver::EnableScreenSaver(HmiData &data, ScreenSaver &saver) : Action(ENABLE_SCREENSAVER_VERB, ENABLE_SCREENSAVER_NOUN, data, ACTION_SINGLE, "SCREENSAVER ON"), saver_(saver)
{
}

void EnableScreenSaver::operation()
{
    LogTrace << "Running ScreenSaver ON" << std::endl;

    saver_.setEnabled(true);
}
