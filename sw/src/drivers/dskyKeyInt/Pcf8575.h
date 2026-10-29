#ifndef _PCF8575_H_
#define _PCF8575_H_

#include <cstdint>

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
     * Pure read: issues a single I2C_M_RD message with no preceding
     * register write. On clone PCF8575 parts a 1-byte register write is
     * latched into the output shift register on STOP and corrupts it,
     * blinding key sensing; a pure read can never disturb the latch.
     *
     * @return int 32-bit word with the GPIO data in the 16 LSB
     */
    int read();

    /**
     * @brief Write the full 16-bit output latch of the expander
     *
     * Only full 2-byte writes may ever be issued: on clone parts a partial
     * write is latched into an arbitrary stable value. Writing 0xFFFF puts
     * every pin into high-impedance input mode, which is the required state
     * for key sensing and self-heals a previously corrupted latch.
     *
     * @param word [in] 16-bit latch value (bit 1 = high-Z input,
     *                  bit 0 = actively drive low)
     */
    void writeLatch(const uint16_t word);
};

#endif /*_PCF8575_H_ */