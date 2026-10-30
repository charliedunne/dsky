
// Standard lib
#include <iostream>
#include <string>

// Configuration
#include "config.h"

// Actions
#include "Action.h"
#include "ActionClock.h"
#include "ScreenSaver.h"
#include "ActionVerbList.h"
#include "SystemReboot.h"
#include "SystemShutdown.h"
#include "ReadVar.h"
#include "WriteVar.h"

// HMI
#include "Hmi.h"
#include "HmiLogic.h"

// Core
#include "Keyboard.h"
#include "Keypad.h"
#include "VariableStore.h"

// Message Queue
#include "MsgQueue.h"
#include "KeyIntMsgDef.h"

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

  // Make stdout line-buffered so journalctl shows log lines in real time
  // instead of after the block buffer fills.
  std::cout << std::unitbuf;

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

  // Instance of the screensaver (constructed early so apply callbacks can use it)
  ScreenSaver screenSaver;

  // Instance of the Keypad
  Keypad keypad;

  // Data to be shared between the logic and the hmi
  HmiData data;

  /* Persistent variable store */
  VariableStore store(VAR_STORE_FILE);

  // NOUN 2: keypad light. Apply re-sends the state to dskyKeyInt via the message queue.
  store.registerVar(VAR_NOUN_KEYPAD_LIGHT, "keypadLight", "KEYPAD LIGHT", VAR_BOOL, 0, [](int value) {
    MsgQueue msgQueue(CMD_MSG_QUEUE_FILE, CMD_MSG_QUEUE_ID, MSG_QUEUE_OUT);
    CommMsg_t cmd;
    cmd.mType = 1;
    cmd.cmd.keylight.enable = (value != 0);
    cmd.cmd.keylight.brightness = (value != 0) ? 255 : 0;
    msgQueue.send(cmd);
  });

  // NOUN 3: screensaver. Apply enables/disables the saver directly.
  store.registerVar(VAR_NOUN_SCREENSAVER, "screensaver", "SCREENSAVER", VAR_BOOL, 1, [&screenSaver](int value) {
    screenSaver.setEnabled(value != 0);
  });

  // Restore persisted values, then re-apply them to hardware.
  store.load();
  store.applyAll();

  // Instance of the logic
  HmiLogic logic(store);

  /* Actions */
  ActionClock a_Clock(data);
  SystemReboot a_SystemReboot(data);
  SystemShutdown a_SystemShutdown(data);
  ReadVar a_ReadKeyLight(store, VAR_NOUN_KEYPAD_LIGHT, data);
  WriteVar a_WriteKeyLight(store, VAR_NOUN_KEYPAD_LIGHT, data);
  ReadVar a_ReadScreenSaver(store, VAR_NOUN_SCREENSAVER, data);
  WriteVar a_WriteScreenSaver(store, VAR_NOUN_SCREENSAVER, data);

  /* Actions registration */
  logic.registerAction(a_Clock);
  logic.registerAction(a_SystemReboot);
  logic.registerAction(a_SystemShutdown);
  logic.registerAction(a_ReadKeyLight);
  logic.registerAction(a_WriteKeyLight);
  logic.registerAction(a_ReadScreenSaver);
  logic.registerAction(a_WriteScreenSaver);

  // VERB 0: list all implemented verb/noun combos on the left panel.
  // Registered last so its menu can describe every other action.
  ActionVerbList a_VerbList(data, logic.getActions(), store);
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

    // Skip rendering while the display is blanked. Presenting with the CRTC
    // disabled leaks Mesa sync_file fence fds (never closed), which exhausts
    // the process fd limit and breaks the screensaver's own sysfs writes.
    if (screenSaver.isScreenOn()) {
      hmi.render();
    }

    extraTime = SDL_framerateDelay(&fpsManager);
    //SDL_Delay(200);

    if (extraTime <= 0) {
      LogError << "Computing time issue with FPS" << std::endl;
    }

  }

  LogInfo << "Shutting down cleanly." << std::endl;
}
