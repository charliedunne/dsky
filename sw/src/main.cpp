
// Standard lib
#include <iostream>
#include <string>

// Configuration
#include "config.h"

// HMI
#include "Hmi.h"

// Core
#include "Keyboard.h"

// Logging
#include <Logger.h>

#include <signal.h>

bool running = true;

void handler(int s) {
  LogInfo << "SIGINT detected" << std::endl;
  running = false;
}

int main( int argc, char * argv[], char *envp[] ) {

  // Initialize core
  LogInfo << "Initializing core...\n";

  // Initialize HMI
  std::srand(std::time(0));

  // Capture signals
  signal(SIGINT, handler);

  Hmi hmi = Hmi(800, 480);

  Keyboard kb("share.txt");

  std::string num;

  HmiData data;

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
    
    kb.update();

    if (kb.eventsAvailable())
    {
      LogInfo << "New Keyboard Event available" << std::endl;
    }

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

    SDL_Delay(200);

  }
}
