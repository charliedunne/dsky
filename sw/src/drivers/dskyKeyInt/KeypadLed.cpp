#include "KeypadLed.h"

// Pigpio
#include "pigpio.h"

KeypadLed::KeypadLed(int gpioPin, int freq, int range) : 
    gpioPin_(gpioPin), pwmFrequency_(freq), range_(range)
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

    for (int i = 0; i < brightness_; ++i)
    {
        gpioPWM(gpioPin_, i);
        gpioDelay(transitionDelay_);
    }
}

void KeypadLed::off(int brightness) 
{
    for (int i = brightness_; i >= 0; --i)
    {
        gpioPWM(gpioPin_, i);
        gpioDelay(transitionDelay_);
    }
    
    brightness_ = brightness;
}
