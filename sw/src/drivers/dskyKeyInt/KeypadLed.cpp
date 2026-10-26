#include "KeypadLed.h"

// Pigpio
#include "pigpio.h"

KeypadLed::KeypadLed(int gpioPin, int freq, int range) :
    gpioPin_(gpioPin), pwmFrequency_(freq), range_(range), brightness_(0)
{
    /** @todo Make configuration parameter */
    transitionDelay_ = 8000; /* 8 ms */

    // Configure Frequency and initial range
    gpioSetPWMfrequency(gpioPin_, pwmFrequency_);
    gpioSetPWMrange(gpioPin_, range_);
}

KeypadLed::~KeypadLed() 
{
    /* Switch off the leds of object destruction */
    off();
}

void KeypadLed::on(int brightness)
{
    brightness_ = brightness;

    /* Ramp up to the target brightness, inclusive */
    for (int i = 0; i <= brightness_; ++i)
    {
        gpioPWM(gpioPin_, i);
        gpioDelay(transitionDelay_);
    }
}

void KeypadLed::off(int brightness)
{
    /* Ramp down to zero. The loop always terminates at PWM 0, so the pin is
     * left off even when this process started while a previous one left the
     * duty cycle on (brightness_ is initialized to 0 in the constructor). */
    for (int i = brightness_; i >= 0; --i)
    {
        gpioPWM(gpioPin_, i);
        gpioDelay(transitionDelay_);
    }

    brightness_ = brightness;
}
