#include <iostream>

// STL
#include <exception>

// System tools
#include <unistd.h>
#include <signal.h>

// KeyInt library
#include "KeyInt.h"

void signal_callback_handler(int signum)
{
    std::cout << "System Signal received (" << signum << ")." << std::endl;

    exit(signum);
}

int main(int arg, char *argv[], char *envp[])
{

    // Register handler for SIGNING
    signal(SIGINT, signal_callback_handler);

    try
    {
        KeyInt keyInt;

        while (true)
        {
            sleep(1);

            /** @todo Polling/blocking call TBC over another message 
             *  queue to detect operation on the HW 
             *
             * @li Enable/Disable Keypad Leds
             * @li Brightness of the screen
             * @li Access to other sensors/actuators (DH111, IMU)
             * @li Shutdown computer
             * @li enable/disable screen?
             */
        }
    } catch (const std::exception &e) {
        std::cerr << "Som error occurr!: " << e.what() << std::endl;
        std::cerr << "Type: " << typeid(e).name() << std::endl;
    }

    return EXIT_SUCCESS;
}
