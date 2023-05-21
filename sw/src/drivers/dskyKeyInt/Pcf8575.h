#ifndef _PCF8575_H_
#define _PCF8575_H_

class Pcf8575
{

private:
    int iicBus_;
    int iicAddr_;

public:
    /**
     * @brief Construct a new Pcf 8 5 7 5 object
     *
     * @param iicBus [in] I2C Bus where the device is connected to
     * @param iicAddr [in] Addres in the I2C bus that identify the device
     */
    Pcf8575(const int iicBus, const int iicAddr)
        : iicBus_(iicBus), iicAddr_(iicAddr) {}

    /**
     * @brief Read the current value of the GPIO lines
     *
     * @return int 32-bit word with the GPIO data in the 16 LSB
     */
    int read();
};

#endif /*_PCF8575_H_ */