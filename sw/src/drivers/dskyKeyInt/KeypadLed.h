#ifndef _KEYPADLED_H_
#define _KEYPADLED_H_

class KeypadLed 
{
    private:

    /**
     * @brief GPIO pin in raspberry where the LEDs are connected to
     */
    int gpioPin_;

    /**
     * @brief PWM Frequency configuration
     */
    int pwmFrequency_;

    /**
     * @brief Transition delay between ON->OFF and OFF->ON
     */
    int transitionDelay_;

    /**
     * @brief PWM Range factor
     */
    int range_;

    /**
     * @brief Brightness [0-255]
     */
    int brightness_;

    public:

    KeypadLed(int gpioPin = 17, int freq = 40, int range = 2000);
    virtual ~KeypadLed();

    void on(int brightness = 255);
    void off(int brightness = 0);
};

#endif /* _KEYPADLED_H_ */