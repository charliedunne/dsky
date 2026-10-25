/* dskyprobe.c — i2c NACK watcher for the two PCF8575 keypad expanders.
 *
 * Polls /dev/i2c-1 directly (independent of any pigpio session, so it can run
 * while dskyKeyInt is active) and performs the same 16-bit register read the
 * daemon uses (i2cReadWordData(handle, 1)): write register byte 0x01, then
 * read two bytes. Counts per-device success/failure with errno, appends a
 * timestamped line to the log file for every failure, and prints a summary.
 *
 * Usage: dskyprobe [duration_s] [logfile]
 *   duration_s  watch window length (default 30)
 *   logfile     failure log path (default /tmp/i2cwatch.log)
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

#define DEV_COUNT 2
#define ADDR_LEFT 0x20
#define ADDR_RIGHT 0x22
#define REG_WORD 0x01 /* register byte for i2cReadWordData(handle, 1) */

/**
 * @brief Perform one 16-bit word read from a PCF8575 via raw i2c-dev.
 * @param fd Open /dev/i2c-N file descriptor.
 * @param addr Slave address (0x20 left, 0x22 right).
 * @return 0 on success, -1 on failure (errno set).
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

    return ioctl(fd, I2C_RDWR, &rdwr);
}

int main(int argc, char *argv[])
{
    int dur = 30;
    if (argc > 1)
    {
        dur = atoi(argv[1]);
    }
    const char *logf = (argc > 2) ? argv[2] : "/tmp/i2cwatch.log";

    FILE *log = fopen(logf, "a");
    if (!log)
    {
        perror("fopen log");
        return 1;
    }

    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0)
    {
        perror("open /dev/i2c-1");
        fclose(log);
        return 1;
    }

    time_t t0 = time(NULL);
    long ok[DEV_COUNT] = {0, 0};
    long fail[DEV_COUNT] = {0, 0};
    int last_errno[DEV_COUNT] = {0, 0};
    char first_fail_ts[DEV_COUNT][32] = {"", ""};

    fprintf(log, "== dskyprobe watch start (dur=%ds) ==\n", dur);
    fflush(log);

    while (time(NULL) - t0 < dur)
    {
        for (int dev = 0; dev < DEV_COUNT; ++dev)
        {
            unsigned char addr = (unsigned char)(ADDR_LEFT + 2 * dev);

            if (readWord(fd, addr) < 0)
            {
                fail[dev]++;
                last_errno[dev] = errno;

                /* Timestamp of this specific failure (for the log line). */
                time_t now = time(NULL);
                struct tm tmv;
                localtime_r(&now, &tmv);
                char ts[32];
                strftime(ts, sizeof ts, "%H:%M:%S", &tmv);

                if (first_fail_ts[dev][0] == '\0')
                {
                    snprintf(first_fail_ts[dev], sizeof first_fail_ts[dev],
                             "%s", ts);
                }
                fprintf(log, "FAIL %s addr=0x%02x errno=%d (%s) at %s\n",
                        dev ? "right" : "left", addr, errno,
                        strerror(errno), ts);
                fflush(log);
            }
            else
            {
                ok[dev]++;
            }
        }
    }

    fprintf(log, "== dskyprobe watch end ==\n");
    fclose(log);
    close(fd);

    printf("watch %d s:\n", dur);
    for (int dev = 0; dev < DEV_COUNT; ++dev)
    {
        long total = ok[dev] + fail[dev];
        double pct = total ? 100.0 * (double)fail[dev] / (double)total : 0.0;
        printf("  %-5s (0x%02x): ok=%ld fail=%ld total=%ld (%.1f%% fail)",
               dev ? "right" : "left", ADDR_LEFT + 2 * dev,
               ok[dev], fail[dev], total, pct);
        if (fail[dev])
        {
            printf(" first_fail=%s last_errno=%d (%s)",
                   first_fail_ts[dev], last_errno[dev], strerror(last_errno[dev]));
        }
        printf("\n");
    }
    return 0;
}
