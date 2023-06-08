
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

bool running = true;

void handler(int s) {
  LogInfo << "SIGINT detected" << std::endl;
  running = false;
}

int main( int argc, char * argv[], char *envp[] ) {

  // Initialize core
  LogInfo << "Initializing core...\n";

  // FPS Manager
  FPSmanager fpsManager;
  int extraTime = 0;

  // Initialize HMI
  std::srand(std::time(0));

  // Capture signals
  signal(SIGINT, handler);

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

  /* Actions registration */
  logic.registerAction(a_Clock);
  logic.registerAction(a_EnableKeypadLight);
  logic.registerAction(a_DisableKeypadLight);

  // Initialize FPS Manager
  SDL_initFramerate(&fpsManager);

  SDL_Event PingStop;

  while (running) {
    //while(SDL_PollEvent(&PingStop)) {
  
    /* Array to register all the keypad events (normally only one per cycle) */
    std::vector<Event> events = keypad.getEvents();

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
}
