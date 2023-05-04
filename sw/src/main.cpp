
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

  while (running) {

    int num = std::rand() % 100;
    std::string n = std::to_string(num);
    //auto str = std::string(2 - std::min(2, ))

    hmi.update(n.c_str());

    hmi.render();

    SDL_Delay(500);
  }

}
