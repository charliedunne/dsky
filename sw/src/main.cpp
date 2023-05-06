
// Standard lib
#include <iostream>
#include <string>

// Configuration
#include "config.h"

// HMI
#include "Hmi.h"

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

  std::string num;

  HmiData data;

  while (running) {

    data.nProg = rand() % 100;
    data.nVerb = rand() % 100;
    data.nNoun = rand() % 100;
    data.nR1 = rand() % 100000;
    data.nR2 = rand() % 100000;
    data.nR3 = rand() % 100000;

    data.fCompActy = rand() % 10;
    data.fProg = rand() % 10;
    data.fVerb = rand() % 10;
    data.fNoun = rand() % 10;
    data.fR1 = rand() % 10;
    data.fR2 = rand() % 10;
    data.fR3 = rand() % 10;
    

    //data.prog = 7;
    
    hmi.update(data);

    hmi.render();

    SDL_Delay(1000);

  }

}
