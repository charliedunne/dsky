
// Standard lib
#include <iostream>
#include <string>

// Configuration
#include "config.h"

// HMI
#include "Hmi.h"

// Core
#include "Keyboard.h"
#include "Keypad.h"

// Logging
#include <Logger.h>

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

  Keypad keypad;

  std::string num;

  HmiData data;

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

    memset(&data, 0x0, sizeof(HmiData));

    data.nProg = 11;
    data.nVerb = 5;
    data.nNoun = 2;
    data.nProgMode = DRAW_ON;
    data.nVerbMode = DRAW_BLINK;
    data.nNounMode = DRAW_BLINK;

    data.lProgMode = DRAW_ON;
    data.lNounMode = DRAW_ON;
    data.lVerbMode = DRAW_ON;

    data.nR1 = 34523;
    data.nR2 = -12438;
    data.nR3 = -7;
    data.nR1Mode = DRAW_ON;
    data.nR2Mode = DRAW_ON;
    data.nR3Mode = DRAW_ON;
    data.lR1Mode = DRAW_ON;
    data.lR2Mode = DRAW_ON;
    data.lR3Mode = DRAW_BLINK;

    hmi.update(data);

    hmi.render();

    extraTime = SDL_framerateDelay(&fpsManager);

    if (extraTime <= 0) {
      LogError << "Computing time issue with FPS" << std::endl;
    }

    //SDL_Delay(200);

  }
}
