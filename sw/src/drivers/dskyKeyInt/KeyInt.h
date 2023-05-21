#ifndef _KEYINT_H_
#define _KEYINT_H_

#include "Pcf8575.h"

class KeyInt
{

private:

    /**
     * @brief Left Keyboard PCF8575 controller
     */
    Pcf8575 * leftKbd_;

    /**
     * @brief Right Keyboard PCF8575 controller
     */
    Pcf8575 * rightKbd_;

    /**
     * @brief Gpio Line where the interruption port is connected to
     */
    int gpioInt_;

    /**
     * @brief Interrupt handler.
     * This funcion will be called on every change in the gpioInt
     * 
     */
    void intHandler();

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
    const int gpioInt);

    /**
     * @brief Destroy the Key Int object
     * 
     */
    virtual ~KeyInt();

};

#endif /*_KEYINT_H_ */