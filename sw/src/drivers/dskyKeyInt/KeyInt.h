#ifndef _KEYINT_H_
#define _KEYINT_H_

#include "Pcf8575.h"

#include "KeyIntMsgDef.h"

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
    static Pcf8575 * leftKbd_;

    /**
     * @brief Right Keyboard PCF8575 controller
     */
    static Pcf8575 * rightKbd_;

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
     * @brief Output Message queue for the output events
    */
   static MsgQueue * outMsgQueue_;
    
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