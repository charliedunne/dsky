/* portdump.c — raw i2c port-value dumper for the two PCF8575 keypad expanders.
 *
 * Polls /dev/i2c-1 directly (independent of any pigpio session, so it can run
 * while dskyKeyInt is active) and prints the actual 16-bit port word for BOTH
 * devices on every tick (~100 ms), so an idle baseline can be diffed against a
 * held-keypress capture to see whether the raw bus sees the key at all.
 *
 * Read modes:
 *   default (daemon-compatible): write register byte 0x01, then read two bytes
 *                                (exactly what pigpio i2cReadWordData(handle,1)
 *                                does on the wire — a 1-byte write + 2-byte read).
 *   -p (pure read): a single I2C_M_RD message reading two bytes with NO
 *                   preceding write. The PCF8575 has no registers; a pure read
 *                   cannot disturb its output latch, so this is the safe
 *                   baseline for A/B testing whether the 1-byte reg write
 *                   corrupts the latch on clone parts.
 *
 * Latch writes:
 *   -w left|right WORD  before polling begins, write a full 16-bit value to
 *                       that device's output latch (low byte first, matching
 *                       the read packing). Any full 2-byte write sets the
 *                       PCF8575 latch; bit=1 means high-Z input. Use this to
 *                       restore or force a known latch state, e.g.
 *                       -w left 0xFFFF puts every left-port pin back into
 *                       input mode.
 *
 * Output is one line per tick, unbuffered, e.g.:
 *   14:02:03.123 left=0x2103 right=0x7055
 *   14:02:03.224 left=FAIL(121) right=0x7055
 * A read failure prints FAIL(errno) for that device on that tick.
 *
 * Usage: portdump [options] [duration_s]
 *   -p                pure-read mode (no register write before each read)
 *   -w left|right W   write 16-bit latch value W to that device first
 *                     (W is parsed with strtoul base 0: 0x for hex)
 *   duration_s        watch window length (default 30)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>      /* struct i2c_msg, I2C_M_RD */
#include <linux/i2c-dev.h>  /* I2C_RDWR, struct i2c_rdwr_ioctl_data */

#define ADDR_LEFT 0x20
#define ADDR_RIGHT 0x22
#define REG_WORD 0x01 /* register byte for i2cReadWordData(handle, 1) */

/**
 * @brief Perform one 16-bit word read from a PCF8575 via raw i2c-dev.
 * @param fd Open /dev/i2c-N file descriptor.
 * @param addr Slave address (0x20 left, 0x22 right).
 * @param out On success, receives the combined 16-bit port word
 *            ((high_byte << 8) | low_byte), matching the daemon's packing.
 * @return 0 on success, -1 on failure (errno set).
 */
static int readWord(int fd, unsigned char addr, unsigned short *out)
{
    unsigned char regBuf[1] = { REG_WORD };
    unsigned char rdbuf[2];

    struct i2c_msg msgs[2] = {
        { .addr = addr, .flags = 0, .len = 1, .buf = regBuf },
        { .addr = addr, .flags = I2C_M_RD, .len = 2, .buf = rdbuf },
    };
    struct i2c_rdwr_ioctl_data rdwr = { .msgs = msgs, .nmsgs = 2 };

    if (ioctl(fd, I2C_RDWR, &rdwr) < 0)
    {
        return -1;
    }
    *out = (unsigned short)((rdbuf[1] << 8) | rdbuf[0]);
    return 0;
}

/**
 * @brief Perform one pure 16-bit word read: a single I2C_M_RD message with no
 *        preceding write. Cannot disturb the PCF8575 output latch.
 * @param fd Open /dev/i2c-N file descriptor.
 * @param addr Slave address (0x20 left, 0x22 right).
 * @param out On success, receives the combined 16-bit port word.
 * @return 0 on success, -1 on failure (errno set).
 */
static int readWordPure(int fd, unsigned char addr, unsigned short *out)
{
    unsigned char rdbuf[2];

    struct i2c_msg msgs[1] = {
        { .addr = addr, .flags = I2C_M_RD, .len = 2, .buf = rdbuf },
    };
    struct i2c_rdwr_ioctl_data rdwr = { .msgs = msgs, .nmsgs = 1 };

    if (ioctl(fd, I2C_RDWR, &rdwr) < 0)
    {
        return -1;
    }
    *out = (unsigned short)((rdbuf[1] << 8) | rdbuf[0]);
    return 0;
}

/**
 * @brief Write a full 16-bit value to the PCF8575 output latch.
 *        Low byte is sent first, matching the read packing order.
 * @param fd Open /dev/i2c-N file descriptor.
 * @param addr Slave address (0x20 left, 0x22 right).
 * @param word 16-bit latch value (bit=1 high-Z input, bit=0 drive low).
 * @return 0 on success, -1 on failure (errno set).
 */
static int writeLatch(int fd, unsigned char addr, unsigned short word)
{
    unsigned char wbuf[2] = { (unsigned char)(word & 0xFF),
                              (unsigned char)((word >> 8) & 0xFF) };

    struct i2c_msg msgs[1] = {
        { .addr = addr, .flags = 0, .len = 2, .buf = wbuf },
    };
    struct i2c_rdwr_ioctl_data rdwr = { .msgs = msgs, .nmsgs = 1 };

    if (ioctl(fd, I2C_RDWR, &rdwr) < 0)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief Format the current time as HH:MM:SS.mmm into buf.
 */
static void nowMs(char *buf, size_t n)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tmv;
    localtime_r(&ts.tv_sec, &tmv);
    snprintf(buf, n, "%02d:%02d:%02d.%03ld", tmv.tm_hour, tmv.tm_min,
             tmv.tm_sec, (long)(ts.tv_nsec / 1000000));
}

int main(int argc, char *argv[])
{
    int dur = 30;
    int pureRead = 0;
    int writeLeft = -1, writeRight = -1; /* <0: no write */

    int i = 1;
    for (; i < argc; ++i)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            pureRead = 1;
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            if (i + 2 >= argc)
            {
                fprintf(stderr, "usage: -w left|right WORD\n");
                return 1;
            }
            unsigned short word = (unsigned short)strtoul(argv[i + 2], NULL, 0);
            if (strcmp(argv[i + 1], "left") == 0)
            {
                writeLeft = word;
            }
            else if (strcmp(argv[i + 1], "right") == 0)
            {
                writeRight = word;
            }
            else
            {
                fprintf(stderr, "-w expects left|right\n");
                return 1;
            }
            i += 2;
        }
        else if (argv[i][0] != '-')
        {
            dur = atoi(argv[i]);
        }
        else
        {
            fprintf(stderr, "unknown option %s\n", argv[i]);
            return 1;
        }
    }

    /* Unbuffered stdout so the capture is real-time even when piped. */
    setvbuf(stdout, NULL, _IONBF, 0);

    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0)
    {
        perror("open /dev/i2c-1");
        return 1;
    }

    /* Perform any requested latch writes before the polling window. */
    if (writeLeft >= 0 && writeLatch(fd, ADDR_LEFT, (unsigned short)writeLeft) < 0)
    {
        fprintf(stderr, "latch write left=0x%04X failed: %s\n",
                (unsigned)writeLeft, strerror(errno));
        close(fd);
        return 1;
    }
    if (writeRight >= 0 &&
        writeLatch(fd, ADDR_RIGHT, (unsigned short)writeRight) < 0)
    {
        fprintf(stderr, "latch write right=0x%04X failed: %s\n",
                (unsigned)writeRight, strerror(errno));
        close(fd);
        return 1;
    }

    struct timespec tick = { 0, 100 * 1000 * 1000 }; /* ~100 ms */
    time_t t0 = time(NULL);

    printf("# portdump dur=%ds mode=%s", dur, pureRead ? "pure-read" : "write-then-read");
    if (writeLeft >= 0)
    {
        printf(" wrote-left=0x%04X", (unsigned)writeLeft);
    }
    if (writeRight >= 0)
    {
        printf(" wrote-right=0x%04X", (unsigned)writeRight);
    }
    printf("\n");
    fflush(stdout);

    while (time(NULL) - t0 < dur)
    {
        char ts[32];
        nowMs(ts, sizeof ts);

        unsigned short lw = 0, rw = 0;
        int lf, rf;
        if (pureRead)
        {
            lf = readWordPure(fd, ADDR_LEFT, &lw);
            rf = readWordPure(fd, ADDR_RIGHT, &rw);
        }
        else
        {
            lf = readWord(fd, ADDR_LEFT, &lw);
            rf = readWord(fd, ADDR_RIGHT, &rw);
        }

        if (lf < 0 && rf < 0)
        {
            printf("%s left=FAIL(%d) right=FAIL(%d)\n", ts, errno, errno);
        }
        else if (lf < 0)
        {
            printf("%s left=FAIL(%d) right=0x%04X\n", ts, errno, rw);
        }
        else if (rf < 0)
        {
            printf("%s left=0x%04X right=FAIL(%d)\n", ts, lw, errno);
        }
        else
        {
            printf("%s left=0x%04X right=0x%04X\n", ts, lw, rw);
        }

        nanosleep(&tick, NULL);
    }

    close(fd);
    return 0;
}
