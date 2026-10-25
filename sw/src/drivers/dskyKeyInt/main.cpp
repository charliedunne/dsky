#include <iostream>

// STL
#include <exception>

// System tools
#include <unistd.h>
#include <csignal>

// KeyInt library
#include "KeyInt.h"
#include "CmdManager.h"
#include "KeypadLed.h"

/* PiGpio */
#include "pigpio.h"

/** @brief Signal number received, set by the handler and read by the main loop. */
static volatile sig_atomic_t g_signalReceived = 0;

/**
 * @brief Signal handler for SIGINT/SIGTERM.
 *
 * Async-signal-safe: only sets the flag. All cleanup (gpioTerminate, exit)
 * happens in the main thread once it observes the flag.
 */
void signal_callback_handler(int signum)
{
    g_signalReceived = signum;
}

int main(int argc, char *argv[])
{

    // Register handlers for SIGINT/SIGTERM with portable sigaction semantics.
    // SA_RESTART is deliberately not set so a blocked msgrcv returns EINTR
    // and the main loop can observe the flag promptly.
    struct sigaction sa = {};
    sa.sa_handler = signal_callback_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    /* libpigpio installs its own handlers for all signals (0..63) during
     * gpioInitialise(), which would clobber the registrations above and turn
     * a clean shutdown into a non-async-signal-safe gpioTerminate()+exit()
     * from signal-handler context. Disable it, as recommended in pigpio.h:
     * "If you intend to rely on signals sent to your application, you should
     * turn off the internal signal handling." */
    int cfg = gpioCfgGetInternals();
    cfg |= PI_CFG_NOSIGHANDLER;
    gpioCfgSetInternals(cfg);

    // Initialize piGpio Library
    gpioInitialise();

    try
    {
        KeyInt keyInt;
        CmdManager cmdManager;
        KeypadLed keypadLed;

        /* Exit the loop as soon as a shutdown signal has been received */
        while (g_signalReceived == 0)
        {
            cmdManager.waitForCommand();

            CommMsg_t cmd = cmdManager.getCommand();

            if (cmd.mType == 1)
            {
                if (cmd.cmd.keylight.enable)
                {
                    keypadLed.on(cmd.cmd.keylight.brightness);
                }
                else
                {
                    keypadLed.off();
                }
            }

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
    }
    catch (const std::exception &e)
    {
        /* A shutdown signal interrupts the blocking msgrcv with EINTR, which
         * surfaces here as an exception. Only report it if no signal was
         * received (i.e. it is a genuine error). */
        if (g_signalReceived == 0)
        {
            std::cerr << "Some error occurr!: " << e.what() << std::endl;
            std::cerr << "Type: " << typeid(e).name() << std::endl;
        }
    }

    // Terminate the GPIO library from the main thread, not the signal handler
    gpioTerminate();

    if (g_signalReceived != 0)
    {
        std::cout << "System Signal received (" << g_signalReceived << ")." << std::endl;
    }

    return EXIT_SUCCESS;
}
