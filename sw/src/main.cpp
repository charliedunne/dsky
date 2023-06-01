
// Standard lib
#include <iostream>
#include <string>

// Configuration
#include "config.h"

// Actions
#include "Action.h"
#include "ActionClock.h"

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

  Hmi hmi = Hmi(800, 480);
  HmiLogic logic;

  Keypad keypad;

  std::string num;

  HmiData data;

  ActionClock a_Clock(&data);
  logic.registerAction(&a_Clock);

  // Initialize FPS Manager
  SDL_initFramerate(&fpsManager);

  while (running) {

    // data.nProg = rand() % 100;
    // data.nVerb = rand() % 100;
    // data.nNoun = rand() % 100;
    // data.nR1 = rand() % 100000;
    // data.nR2 = rand() % 100000;
    // data.nR3 = rand() % 100000;

    // data.fCompActy = rand() % 10;
    // data.fProg = rand() % 10;
    // data.fVerb = rand() % 10;
    // data.fNoun = rand() % 10;
    // data.fR1 = rand() % 10;
    // data.fR2 = rand() % 10;
    // data.fR3 = rand() % 10;
    
    std::vector<Event> events = keypad.getEvents();

    logic.parseKeyEvents(events);

    for (int i = 0; i < events.size(); ++i) {

      std::cout << "[" << i << "] Keystroke Received: " << std::hex << events[i].getEvent() << std::endl;
    }

    /** @todo Call core function to process environment before rendering.
     * it will contain:
     * @li List of keypad events
     * @li 
     * 
     * The output of this function will be the HmiData struct
     */

    //    memset(&data, 0x0, sizeof(HmiData));


    /* Call the logic updated */
    logic.updateHmiData(&data);

    /**
     * @todo Get feedback from logic and send message queue to another
     * application that will run the actions.
     *
     * Also it would be required some data from the application to the
     * HMI
     *
     * Other option is to create and run an independent thread
     * that will make the operation. it will run in parallel and we
     * can use some synchronization method to report data.
     *
     * E.G: CLOCK:
     * @li Required:
     *   @li update every second the time
     *   
     */

    hmi.update(data);

    hmi.render();

    extraTime = SDL_framerateDelay(&fpsManager);

    if (extraTime <= 0) {
      LogError << "Computing time issue with FPS" << std::endl;
    }

    //SDL_Delay(200);

  }
}
