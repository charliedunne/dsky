
// Standard lib
#include <iostream>

// Logging
#include "BSlogger.hpp"

// Configuration
#include "config.h"

// HMI
#include "Hmi.h"

bool running = true;

int main( int argc, char * argv[], char *envp[] ) {

  // Initialize logging system
  LOG_INIT_CLOG();
  log.set_log_level(LOG_LEVEL);

  // Initialize core
  log(LOG_INFO) << "Initializing core...\n";
  log(LOG_INFO) << "TBD\n";


  // Initialize HMI
  log(LOG_INFO) << "Initializing HMI...\n";
  
  Hmi hmi;

  log(LOG_INFO) << "TBD\n";

  // Main loop
  //  while (running) {

    // Capture events

    // Update status

    // Render

    // Wait for next cycle
  //  }

}
