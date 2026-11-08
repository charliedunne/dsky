
// Standard lib
#include <iostream>
#include <string>
#include <cstdio>

// Configuration
#include "config.h"

// Actions
#include "Action.h"
#include "ActionClock.h"
#include "ActionVerbList.h"
#include "SystemReboot.h"
#include "SystemShutdown.h"
#include "ReadVar.h"
#include "WriteVar.h"
#include "TempMonitor.h"
#include "StopBgProg.h"
#include "ListBgProg.h"

// Programs
#include "DummyProgram.h"
#include "DbMonitorProgram.h"
#include "MoonPhaseProgram.h"
#include "UdpDoorProgram.h"
#include "ScreensaverProgram.h"

// Database client (live temp sensor data for VERB 10)
#include "DbClient.h"

// HMI
#include "Hmi.h"
#include "HmiLogic.h"

// Core
#include "Keyboard.h"
#include "Keypad.h"
#include "VariableStore.h"
#include "DskyVariables.h"

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

  // Line-buffer stdout at the C level (FILE*). The logger writes through a
  // custom streambuf that sputc()s straight into cout's underlying filebuf,
  // so std::unitbuf on std::cout never triggers: no << operator is ever
  // invoked on cout itself. Under journald (pipe) glibc would otherwise
  // block-buffer stdout (~4 KB), delaying log lines until the buffer fills
  // or the process exits. setvbuf must run before any output.
  setvbuf(stdout, NULL, _IOLBF, 0);

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

  // Instance of the Keypad
  Keypad keypad;

  // Data to be shared between the logic and the hmi
  HmiData data;

  /* Persistent variable store */
  VariableStore store(VAR_STORE_FILE);

  // Register all DSKY variables (names, labels, types, defaults) in one place.
  DskyVariables dskyVars(store);

  // Instance of the screensaver (constructed early so apply callbacks can use it;
  // needs the HMI data and the variable store as constructor arguments)
  ScreensaverProgram p_Screensaver(data, store);

  // NOUN 2: keypad light. Apply re-sends the state to dskyKeyInt via the message queue.
  store.setApply(DskyVariables::NOUN_KEYPAD_LIGHT, [](int value) {
    MsgQueue msgQueue(CMD_MSG_QUEUE_FILE, CMD_MSG_QUEUE_ID, MSG_QUEUE_OUT);
    CommMsg_t cmd;
    cmd.mType = 1;
    cmd.cmd.keylight.enable = (value != 0);
    cmd.cmd.keylight.brightness = (value != 0) ? 255 : 0;
    msgQueue.send(cmd);
  });

  // NOUN 4: screensaver idle timer [seconds]. Apply updates the running timer.
  store.setApply(DskyVariables::NOUN_SCREENSAVER_TIMER, [&p_Screensaver](int value) {
    p_Screensaver.setTimer(value);
  });

  // Restore persisted values, then re-apply them to hardware.
  store.load();
  store.applyAll();

  // Instance of the logic
  HmiLogic logic(store);

  // Background DB client for live temperature sensor data (VERB 10).
  // Runs its own refresh thread; a missing/unreadable config is non-fatal —
  // TempMonitor simply shows no data until a successful fetch occurs.
  DbClient dbClient;
  if (!dbClient.loadConfig(DB_CONF_FILE)) {
    LogError << "Failed to load DB config from " << DB_CONF_FILE << "; temperature monitor will have no data" << std::endl;
  } else {
    dbClient.start();
  }

  /* Actions */
  ActionClock a_Clock(data);
  SystemReboot a_SystemReboot(data);
  SystemShutdown a_SystemShutdown(data);
  ReadVar a_ReadVar(store, data);
  WriteVar a_WriteVar(store, data);
  TempMonitor a_TempMonitor(data, dbClient);

  /* Programs */
  DummyProgram p_Dummy(data);
  DbMonitorProgram p_DbMonitor(data, store, dbClient);
  MoonPhaseProgram p_MoonPhase(data);
  /* Door state changes are screen-worthy events: the program wakes the
   * screensaver so an alarm is never raised into a blanked display. */
  UdpDoorProgram p_UdpDoor(data, p_Screensaver);

  /* Actions registration */
  logic.registerAction(a_Clock);
  logic.registerAction(a_SystemReboot);
  logic.registerAction(a_SystemShutdown);
  logic.registerAction(a_ReadVar);
  logic.registerAction(a_WriteVar);
  logic.registerAction(a_TempMonitor);

  /* Programs registration */
  logic.registerProgram(p_Dummy);
  logic.registerProgram(p_DbMonitor);
  logic.registerProgram(p_MoonPhase);
  logic.registerProgram(p_UdpDoor);
  logic.registerProgram(p_Screensaver);

  /* Boot-time auto-start of background programs, controlled by DSKY variables.
   * All are boot-only: the values are read once here and changing them later
   * (VERB 9) takes effect on the next boot. A program whose autostart flag is
   * 0 can still be started manually from the keypad at any time. Note that if
   * DbMonitor is not auto-started, the TEMP alarm stays dead until it is
   * started manually, since HmiLogic derives that label from dbHealthy_. */
  int dummyAutostart = 0;
  if (store.get(DskyVariables::NOUN_DUMMY_AUTOSTART, dummyAutostart) && dummyAutostart != 0) {
    p_Dummy.start();
  }
  int dbMonitorAutostart = 0;
  if (store.get(DskyVariables::NOUN_DBMONITOR_AUTOSTART, dbMonitorAutostart) && dbMonitorAutostart != 0) {
    p_DbMonitor.start();
  }
  /* Local fallback matches the registered variable default (1 = autostart on) */
  int udpdoorAutostart = 1;
  if (store.get(DskyVariables::NOUN_UDPDOOR_AUTOSTART, udpdoorAutostart) && udpdoorAutostart != 0) {
    p_UdpDoor.start();
  }
  /* Local fallback matches the registered variable default (1 = autostart on) */
  int screensaverAutostart = SCREENSAVER_AUTOSTART_DEFAULT;
  if (store.get(DskyVariables::NOUN_SCREENSAVER_AUTOSTART, screensaverAutostart) && screensaverAutostart != 0) {
    p_Screensaver.start();
  }

  /* VERB 7: stop a running background program selected via the noun field
   * (+/- cycling or typed ID). Needs the program registry, so it is
   * instantiated after the programs are registered. */
  StopBgProg a_StopBg(logic.getPrograms(), data);
  /* Select the target program through the noun input before running */
  a_StopBg.setNeedsTypedNoun(true);
  logic.registerAction(a_StopBg);

  /* VERB 6: list currently running background programs on the left LCD.
   * Verb-only action; needs the program registry like StopBgProg above. */
  ListBgProg a_ListBg(logic.getPrograms(), data);
  logic.registerAction(a_ListBg);

  // VERB 0: list all implemented verb/noun combos on the left panel.
  // Registered last so its menu can describe every other action.
  ActionVerbList a_VerbList(data, logic.getActions(), store, logic.getPrograms());
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
      p_Screensaver.notifyActivity();
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
    if (p_Screensaver.isScreenOn()) {
      hmi.render();
    }

    extraTime = SDL_framerateDelay(&fpsManager);
    //SDL_Delay(200);

    if (extraTime <= 0) {
      LogError << "Computing time issue with FPS" << std::endl;
    }

  }

  // Stop the DB refresh thread before actions are stopped so no consumer can
  // observe a half-torn-down cache during shutdown.
  dbClient.stop();

  // Stop all actions before they are destroyed below. A periodic action that
  // is still running can wake up during destruction and virtually dispatch
  // operation() against the already-restored base vtable, hitting a
  // pure-virtual terminator and aborting the process.
  logic.getActions().stopAll();

  // Stop all programs before they are destroyed below, for the same reason
  // as actions above.
  logic.getPrograms().stopAll();

  LogInfo << "Shutting down cleanly." << std::endl;
}
