#include <iostream>

// STL
#include <exception>

// System tools
#include <unistd.h>
#include <csignal>
#include <pthread.h>

// KeyInt library
#include "KeyInt.h"
#include "CmdManager.h"
#include "KeypadLed.h"

/* PiGpio */
#include "pigpio.h"

/** @brief Signal number received, set by the handler and read by the main loop. */
static volatile sig_atomic_t g_signalReceived = 0;

/**
 * @brief Native ID of the main thread, captured at startup.
 *
 * Used by the signal handler to re-target the signal at the main thread so
 * that its blocking msgrcv() is interrupted with EINTR.
 */
static pthread_t g_mainThreadId = 0;

/**
 * @brief Signal handler for SIGINT/SIGTERM.
 *
 * Async-signal-safe: sets the flag and, when running on a non-main thread,
 * re-targets the signal at the main thread (pthread_self/pthread_kill are
 * async-signal-safe). A process-directed signal may be delivered to any
 * unblocked thread; only a signal received by the main thread itself can
 * interrupt its blocking msgrcv() with EINTR.
 *
 * IMPORTANT: the re-target must NOT happen when this handler already runs on
 * the main thread. A self-directed pthread_kill from inside the handler is
 * not a no-op: the signal is blocked in the running thread while its own
 * handler executes, so the kill merely pends it and the kernel re-invokes
 * this handler forever once the handler returns (infinite re-entrancy; the
 * main thread spins at 100% CPU and msgrcv never resumes).
 * All cleanup (gpioTerminate, exit) happens in the main thread once it
 * observes the flag.
 */
void signal_callback_handler(int signum)
{
    g_signalReceived = signum;
    if (!pthread_equal(pthread_self(), g_mainThreadId)) {
        pthread_kill(g_mainThreadId, signum);
    }
}

int main(int argc, char *argv[])
{
    // Capture the main thread ID before any handler can fire (sigaction is
    // installed below), so the handler always has a valid target.
    g_mainThreadId = pthread_self();

    // Make stdout unit-buffered: under journald (pipe) std::cout would be
    // block-buffered and KeyId lines would only appear on process exit,
    // making live journal verification impossible.
    std::cout << std::unitbuf;

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
