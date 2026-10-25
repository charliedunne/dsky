/*
 * cmdsend.cpp — standalone sender of CommMsg_t commands to the dskyKeyInt
 * CMD message queue (bypasses the HMI app).
 *
 * Used for daemon testing and end-to-end regression: injects exactly the
 * same command messages the app would send, without running the app.
 *
 * Usage:
 *   cmdsend on [brightness]   enable keypad light (default brightness 255)
 *   cmdsend off               disable keypad light
 *   cmdsend status            show CMD queue key/qid and pending message count
 *
 * The queue is identified by ftok("/var/local/dskycmd", 12) — the same key
 * the app uses (see config.h.in). The marker file must exist and dskyKeyInt
 * must be running (it owns the queue); cmdsend never creates the queue.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/msg.h>
#include <sys/stat.h>

// Single source of truth for the message layout (shared with app + daemon)
#include "../KeyIntMsgDef.h"

#define CMD_MSG_QUEUE_FILE "/var/local/dskycmd"
#define CMD_MSG_QUEUE_ID   12

/**
 * @brief Resolve the CMD queue key via ftok.
 * @return The queue key on success.
 * @note Exits with an error message if the marker file is missing or ftok fails.
 */
static long cmdQueueKey()
{
    struct stat st;

    if (stat(CMD_MSG_QUEUE_FILE, &st) != 0) {
        fprintf(stderr, "cmdsend: cannot stat %s: %s\n",
                CMD_MSG_QUEUE_FILE, strerror(errno));
        exit(1);
    }

    long key = ftok(CMD_MSG_QUEUE_FILE, CMD_MSG_QUEUE_ID);
    if (key == -1) {
        fprintf(stderr, "cmdsend: ftok(%s, %d) failed: %s\n",
                CMD_MSG_QUEUE_FILE, CMD_MSG_QUEUE_ID, strerror(errno));
        exit(1);
    }

    return key;
}

/**
 * @brief Open the existing CMD queue (never creates it).
 * @param key Queue key from cmdQueueKey().
 * @return Message queue id on success.
 * @note Exits with an error message if the queue does not exist yet.
 */
static int openCmdQueue(long key)
{
    int qid = msgget(key, 0);
    if (qid == -1) {
        fprintf(stderr, "cmdsend: msgget(0x%lx) failed: %s — is dskyKeyInt running?\n",
                (unsigned long)key, strerror(errno));
        exit(1);
    }

    return qid;
}

/**
 * @brief Send a keylight command to the CMD queue.
 * @param enable     1 = light on, 0 = light off.
 * @param brightness PWM brightness used when enabling (0 when disabling).
 */
static void sendKeylight(int enable, unsigned int brightness)
{
    long key = cmdQueueKey();
    int qid = openCmdQueue(key);

    CommMsg_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.mType = 1;
    cmd.cmd.keylight.enable = (bool)enable;
    cmd.cmd.keylight.brightness = brightness;

    // msgsnd size excludes the leading mtype long
    if (msgsnd(qid, &cmd, sizeof(cmd) - sizeof(long), 0) != 0) {
        fprintf(stderr, "cmdsend: msgsnd failed: %s\n", strerror(errno));
        exit(1);
    }

    printf("sent keylight{enable=%d, brightness=%u} to queue key=0x%lx\n",
           enable, brightness, (unsigned long)key);
}

/**
 * @brief Print CMD queue identity and pending message count.
 */
static void showStatus()
{
    long key = cmdQueueKey();
    int qid = openCmdQueue(key);

    struct msqid_ds info;
    if (msgctl(qid, IPC_STAT, &info) != 0) {
        fprintf(stderr, "cmdsend: msgctl(IPC_STAT) failed: %s\n", strerror(errno));
        exit(1);
    }

    printf("queue key : 0x%lx (%s, id %d)\n", (unsigned long)key,
           CMD_MSG_QUEUE_FILE, CMD_MSG_QUEUE_ID);
    printf("qid       : %d\n", qid);
    printf("pending   : %lu message(s) — should be 0 while dskyKeyInt is running\n",
           (unsigned long)info.msg_qnum);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr,
                "Usage: %s on [brightness] | off | status\n"
                "  on [b]   send keylight enable command (default b=255)\n"
                "  off      send keylight disable command\n"
                "  status   show CMD queue key/qid and pending message count\n",
                argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "on") == 0) {
        unsigned int brightness = 255;
        if (argc > 2) {
            char *end = NULL;
            long v = strtol(argv[2], &end, 10);
            if (end == argv[2] || *end != '\0' || v < 0 || v > 255) {
                fprintf(stderr, "cmdsend: brightness must be 0..255, got '%s'\n", argv[2]);
                return 1;
            }
            brightness = (unsigned int)v;
        }
        sendKeylight(1, brightness);
    } else if (strcmp(argv[1], "off") == 0) {
        sendKeylight(0, 0);
    } else if (strcmp(argv[1], "status") == 0) {
        showStatus();
    } else {
        fprintf(stderr, "cmdsend: unknown command '%s' (use on/off/status)\n", argv[1]);
        return 1;
    }

    return 0;
}
