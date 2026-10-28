
// Standard lib
#include <iostream>
#include <string>

// Configuration
#include "config.h"

// Actions
#include "Action.h"
#include "ActionClock.h"
#include "EnableKeypadLight.h"
#include "DisableKeypadLight.h"
#include "ScreenSaver.h"
#include "EnableScreenSaver.h"
#include "DisableScreenSaver.h"
#include "ActionVerbList.h"
#include "SystemShutdown.h"

// HMI
#include "Hmi.h"
#include "HmiLogic.h"

// Core
#include "Keyboard.h"
#include "Keypad.h"

// Logging
#include <Logger.h>

// Signal handling
#include <signal.h>

// GFX
#include "SDL2_framerate.h"

// volatile sig_atomic_t is the only type guaranteed safe to set in a signal
// handler and read elsewhere. The logger is NOT async-signal-safe (it takes a
// mutex and allocates memory), so we must not call it from here.
volatile sig_atomic_t running = 1;

void handler(int s) {
  (void)s;
  running = 0;
}

int main( int argc, char * argv[], char *envp[] ) {

  // Initialize core
  LogInfo << "Initializing core...\n";

  // FPS Manager
  FPSmanager fpsManager;
  int extraTime = 0;

  // Initialize HMI
  std::srand(std::time(0));

  // Capture signals (sigaction is preferred over signal for portable semantics).
  // SIGTERM as well so `systemctl stop` exits cleanly and flushes the log.
  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGTERM, &sa, nullptr);

  // Instanciation of the HMI
  Hmi hmi = Hmi(800, 480);

  // Instance of the logic
  HmiLogic logic;

  // Instance of the Keypad
  Keypad keypad;

  std::string num;

  // Data to be shared between the logic and the hmi
  HmiData data;

  /* Actions */
  ActionClock a_Clock(data);
  EnableKeypadLight a_EnableKeypadLight(data);
  DisableKeypadLight a_DisableKeypadLight(data);
  ScreenSaver screenSaver;
  EnableScreenSaver a_EnableScreenSaver(data, screenSaver);
  DisableScreenSaver a_DisableScreenSaver(data, screenSaver);
  SystemShutdown a_SystemShutdown(data);

  /* Actions registration */
  logic.registerAction(a_Clock);
  logic.registerAction(a_EnableKeypadLight);
  logic.registerAction(a_DisableKeypadLight);
  logic.registerAction(a_EnableScreenSaver);
  logic.registerAction(a_DisableScreenSaver);
  logic.registerAction(a_SystemShutdown);

  // VERB 0: list all implemented verb/noun combos on the left panel.
  // Registered last so its menu can describe every other action.
  ActionVerbList a_VerbList(data, logic.getActions());
  logic.registerAction(a_VerbList);

  // Initialize FPS Manager
  SDL_initFramerate(&fpsManager);

  SDL_Event PingStop;

  while (running) {
    //while(SDL_PollEvent(&PingStop)) {

    /* Array to register all the keypad events (normally only one per cycle) */
    std::vector<Event> events = keypad.getEvents();

    // Any keypad activity resets the screensaver idle timer and wakes the screen.
    if (!events.empty()) {
      screenSaver.noteInteraction();
    }

    // Re-check after a blocking call so Ctrl+C stops us at the next point
    if (!running) break;

    /* Parse the key events in the logic */
    logic.parseKeyEvents(events);

    for (int i = 0; i < events.size(); ++i) {

      std::cout << "[" << i << "] Keystroke Received: " << std::hex << events[i].getEvent() << std::endl;
    }

    /* Call the logic updated */
    logic.updateHmiData(data);

    hmi.update(data);

    hmi.render();

    extraTime = SDL_framerateDelay(&fpsManager);
    //SDL_Delay(200);

    if (extraTime <= 0) {
      LogError << "Computing time issue with FPS" << std::endl;
    }

  }

  LogInfo << "Shutting down cleanly." << std::endl;
}
