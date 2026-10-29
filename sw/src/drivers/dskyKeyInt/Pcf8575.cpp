// Provided interface
#include "Pcf8575.h"

// STL
#include <cstdio>
#include <stdexcept>

// System
#include <fcntl.h>
#include <linux/i2c.h>       /* struct i2c_msg, I2C_M_RD */
#include <linux/i2c-dev.h>   /* I2C_RDWR ioctl */
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

/**
 * @brief Open the i2c bus device node for this expander.
 *
 * @param iicBus [in] I2C bus number (e.g. 1 -> /dev/i2c-1)
 * @return Opened file descriptor, or -1 on failure (errno set).
 */
int openI2cBus(const int iicBus)
{
    char path[32];
    std::snprintf(path, sizeof(path), "/dev/i2c-%d", iicBus);
    return ::open(path, O_RDWR);
}

} // namespace

int Pcf8575::read()
{

    // Output data
    int data = 0;

    // Open the i2c communication.
    // NOTE: this driver intentionally talks to the kernel i2c-dev interface
    // directly instead of pigpio's i2cReadWordData(). The latter issues a
    // 1-byte register write before every read; on clone PCF8575 parts that
    // partial write is latched into the output shift register on STOP and
    // corrupts it, blinding key sensing. A pure read can never disturb the
    // latch.
    const int fd = openI2cBus(iicBus_);

    if (fd < 0)
    {

        throw std::domain_error("open(/dev/i2c-N) Error");
    }

    // Single pure-read message: two bytes, no preceding write.
    unsigned char rdbuf[2];
    struct i2c_msg msgs[1] = {
        { .addr = (unsigned char)iicAddr_, .flags = I2C_M_RD, .len = 2,
          .buf = rdbuf },
    };
    struct i2c_rdwr_ioctl_data rdwr = { .msgs = msgs, .nmsgs = 1 };

    const bool ok = (ioctl(fd, I2C_RDWR, &rdwr) >= 0);

    // Close before any error reporting so the fd is never leaked on the
    // exception path.
    close(fd);

    if (!ok)
    {

        throw std::domain_error("i2c pure read Error");
    }

    data = (rdbuf[1] << 8) | rdbuf[0];

    return data;
}

void Pcf8575::writeLatch(const uint16_t word)
{

    // Open the i2c communication.
    const int fd = openI2cBus(iicBus_);

    if (fd < 0)
    {

        throw std::domain_error("open(/dev/i2c-N) Error");
    }

    // Full 16-bit latch write, low byte first (matches the read packing
    // order). Only full 2-byte writes may ever be issued: on clone parts a
    // partial write is latched into an arbitrary stable value. Writing
    // 0xFFFF puts every pin into high-impedance input mode, which is the
    // required state for key sensing and self-heals a corrupted latch.
    unsigned char wbuf[2] = { (unsigned char)(word & 0xFF),
                              (unsigned char)((word >> 8) & 0xFF) };

    struct i2c_msg msgs[1] = {
        { .addr = (unsigned char)iicAddr_, .flags = 0, .len = 2, .buf = wbuf },
    };
    struct i2c_rdwr_ioctl_data rdwr = { .msgs = msgs, .nmsgs = 1 };

    const bool ok = (ioctl(fd, I2C_RDWR, &rdwr) >= 0);

    // Close before any error reporting so the fd is never leaked on the
    // exception path.
    close(fd);

    if (!ok)
    {

        throw std::domain_error("i2c latch write Error");
    }
}
