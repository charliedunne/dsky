#ifndef _KEYINT_H_
#define _KEYINT_H_

#include "Pcf8575.h"

#include "KeyIntMsgDef.h"

// STL
#include <cstdint>
#include <mutex>

// Message queues
#include "MsgQueue.h"

/* PiGpio */
#include "pigpio.h"

class KeyInt
{

private:
    /**
     * @brief Left Keyboard PCF8575 controller
     */
    static Pcf8575 *leftKbd_;

    /**
     * @brief Right Keyboard PCF8575 controller
     */
    static Pcf8575 *rightKbd_;

    /**
     * @brief Gpio Line where the interruption port is connected to
     */
    static int gpioInt_;

    /**
     * @brief Interrupt handler.
     * This funcion will be called on every change in the gpioInt
     *
     */
    static void intHandler(int gpio, int level, uint32_t tick);

    /**
     * @brief Periodic polling fallback handler.
     * Called by a pigpio timer; detects key changes even when the INT line
     * is open or intermittent (hardware fault on the shared INT net).
     */
    static void pollHandler(void);

    /**
     * @brief Process a freshly read keyboard state.
     * Emits a KeyId message only if the state differs from the last
     * processed one and at least one key is pressed. Shared by the INT
     * edge path and the polling fallback so they cannot double-emit.
     */
    static void processState(int leftData, int rightData);

    /**
     * @brief Last processed left keyboard state (dedup between paths)
     */
    static int lastLeft_;

    /**
     * @brief Last processed right keyboard state (dedup between paths)
     */
    static int lastRight_;

    /**
     * @brief Debounced (accepted) left keyboard state. A bit only flips
     * in this value after the raw line has held the new level for
     * KEY_DEBOUNCE_MS continuously, so chattering marginal pins never
     * produce events.
     */
    static int debLeft_;

    /**
     * @brief Debounced (accepted) right keyboard state (see debLeft_)
     */
    static int debRight_;

    /**
     * @brief Per-bit debounce timers for the left port, in milliseconds of
     * steady_clock. 0 means the bit is currently stable; any other value
     * is the timestamp at which the bit was first seen in its new (not yet
     * accepted) level.
     */
    static uint32_t leftDebounceMs_[16];

    /**
     * @brief Per-bit debounce timers for the right port (see leftDebounceMs_)
     */
    static uint32_t rightDebounceMs_[16];

    /**
     * @brief True once the debounced state has been seeded from the first
     * successful read (prevents a spurious startup emission)
     */
    static bool seeded_;

    /**
     * @brief Mutex protecting the last-state pair and the emit path
     * (INT alert thread and poll timer thread both call processState)
     */
    static std::mutex stateMutex_;

    /**
     * @brief Output Message queue for the output events
     */
    static MsgQueue *outMsgQueue_;

public:
    /**
     * @brief Construct a new Key Int object
     *
     * @param iicBus [in] I2c Bus identifier (default 1)
     * @param iicAddrLeft [in] I2c address for the left Keyboard (default 0x20)
     * @param iicAddrRigth [in] I2c address for the right Keyboard (default 0x22)
     * @param gpioInt [in] gpio signal where the interrupt line will be connected to
     */
    KeyInt(const int iicBus = 1,
           const int iicAddrLeft = 0x20, const int iicAddrRigth = 0x22,
           const int gpioInt = 4);

    /**
     * @brief Destroy the Key Int object
     *
     */
    virtual ~KeyInt();

private:
    KeyMsg_t buildMessage(int leftKe);
};

#endif /*_KEYINT_H_ */