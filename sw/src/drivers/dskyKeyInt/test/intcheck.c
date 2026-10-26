/* intcheck.c — live INT-line + expander sampler for the PCF8575 keypad.
 *
 * Samples GPIO4 (the shared interrupt line, open-drain active-LOW, idle HIGH)
 * via sysfs and reads both keypad expanders on /dev/i2c-1 using the same 16-bit
 * register read the daemon performs (write register byte 0x01, then read two
 * bytes). Emits one timestamped line per sample so a key hold can be correlated
 * with the INT line asserting LOW. Runs independently of any pigpio session, so
 * it is safe to run while dskyKeyInt is active.
 *
 * Usage: intcheck [duration_s] [logfile]
 *   duration_s  sampling window length in seconds (default 95)
 *   logfile     output log path (default /tmp/intcheck.log)
 *
 * Sample line format: <epoch_ms> g4=<0|1> L=<dec> R=<dec>
 *   g4=0 means the INT line is asserted (LOW). A held key shows as a non-65535
 *   word on its expander (VERB -> L=32767, ENTR -> R=65503).
 */

#define _GNU_SOURCE /* expose clock_gettime / usleep under strict -std=c11 */

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
#define GPIO_INT 4    /* shared INT line from both expanders */
#define SAMPLE_MS 150 /* ~6.7 samples per second */

static const char *GPIO_EXPORT = "/sys/class/gpio/export";
static const char *GPIO_UNEXPORT = "/sys/class/gpio/unexport";
static const char *GPIO_DIR_FMT = "/sys/class/gpio/gpio%d/direction";
static const char *GPIO_VAL_FMT = "/sys/class/gpio/gpio%d/value";

/**
 * @brief Write a string to a sysfs file, ignoring errors (e.g. EEXIST).
 * @param path Full sysfs file path.
 * @param val NUL-terminated value to write.
 */
static void sysfsWrite(const char *path, const char *val)
{
    int fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        return; /* already exported / not present: harmless */
    }
    (void)!write(fd, val, strlen(val));
    close(fd);
}

/**
 * @brief Export a GPIO for sysfs access and set it as an input.
 * @param gpio GPIO number to export.
 */
static void gpioExport(int gpio)
{
    char buf[16];
    char dirpath[64];
    snprintf(buf, sizeof buf, "%d", gpio);
    sysfsWrite(GPIO_EXPORT, buf); /* EEXIST if already exported: fine */
    snprintf(dirpath, sizeof dirpath, GPIO_DIR_FMT, gpio);
    sysfsWrite(dirpath, "in"); /* matches daemon config; electrically a no-op */
}

/**
 * @brief Read the current level of an already-opened sysfs GPIO value file.
 * @param fd Open /sys/class/gpio/gpioN/value descriptor.
 * @return 1 if HIGH, 0 if LOW, -1 on read error.
 */
static int gpioRead(int fd)
{
    char c = 0;
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &c, 1) != 1)
    {
        return -1;
    }
    return (c == '1') ? 1 : 0;
}

/**
 * @brief Perform one 16-bit word read from a PCF8575 via raw i2c-dev.
 * @param fd Open /dev/i2c-N file descriptor.
 * @param addr Slave address (0x20 left, 0x22 right).
 * @return The 16-bit input word, or -1 on failure (errno set).
 */
static int readWord(int fd, unsigned char addr)
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
    /* Combine in the daemon's pigpio i2cReadWordData byte order so displayed
     * values match the KeyId signatures exactly (VERB -> L=0x7FFF). */
    return (int)(((unsigned)rdbuf[1] << 8) | rdbuf[0]);
}

/**
 * @brief Return the current wall-clock time in epoch milliseconds.
 */
static long long nowMs(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

int main(int argc, char *argv[])
{
    int dur = 95;
    if (argc > 1)
    {
        dur = atoi(argv[1]);
    }
    const char *logf = (argc > 2) ? argv[2] : "/tmp/intcheck.log";

    /* Export + open the INT GPIO for sysfs reads. */
    gpioExport(GPIO_INT);
    char valpath[64];
    snprintf(valpath, sizeof valpath, GPIO_VAL_FMT, GPIO_INT);
    int gfd = open(valpath, O_RDONLY);
    if (gfd < 0)
    {
        perror("open gpio value");
        return 1;
    }

    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0)
    {
        perror("open /dev/i2c-1");
        close(gfd);
        return 1;
    }

    FILE *log = fopen(logf, "w");
    if (!log)
    {
        perror("fopen log");
        close(fd);
        close(gfd);
        return 1;
    }

    /* Baseline sanity read for the header line. */
    int blL = readWord(fd, ADDR_LEFT);
    int blR = readWord(fd, ADDR_RIGHT);
    int blG = gpioRead(gfd);
    fprintf(log, "start %lld g4=%d L=%d R=%d\n", nowMs(), blG, blL, blR);
    fflush(log);

    long long tEnd = nowMs() + (long long)dur * 1000LL;
    while (nowMs() < tEnd)
    {
        int g = gpioRead(gfd);
        int L = readWord(fd, ADDR_LEFT);
        int R = readWord(fd, ADDR_RIGHT);
        fprintf(log, "%lld g4=%d L=%d R=%d\n", nowMs(), g, L, R);
        fflush(log);
        usleep(SAMPLE_MS * 1000);
    }

    fprintf(log, "end %lld\n", nowMs());
    fclose(log);
    close(fd);
    close(gfd);

    /* Restore pre-test state: the daemon uses pigpio, not sysfs, so unexporting
     * is safe and leaves the system as we found it. */
    sysfsWrite(GPIO_UNEXPORT, "4");

    printf("intcheck done: %d s -> %s\n", dur, logf);
    return 0;
}
