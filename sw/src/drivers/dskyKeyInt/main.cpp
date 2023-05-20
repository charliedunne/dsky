#include <iostream>

// System tools
#include <unistd.h>
#include <signal.h>

void signal_callback_handler(int signum) {
    std::cout << "System Signal received (" << signum << ")." << std::endl;

    exit(signum); 
}


int main( int arg, char * argv[], char *envp[] ) {

    // Register handler for SIGNING
    signal(SIGINT, signal_callback_handler);

    while (true) {
        sleep(1);
    }

    return EXIT_SUCCESS;
}
