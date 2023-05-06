
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

    data.prog = rand() % 100;
    data.verb = rand() % 100;
    data.noun = rand() % 100;
    data.r1 = rand() % 100000;
    data.r2 = rand() % 100000;
    data.r3 = rand() % 100000;

    //data.prog = 7;
    
    hmi.update(data);

    hmi.render();

    SDL_Delay(1000);

  }

}
