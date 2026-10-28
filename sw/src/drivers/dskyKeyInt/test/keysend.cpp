/*
 * keysend.cpp — standalone sender of KeyMsg_t keystrokes to the dsky
 * input message queue (bypasses the dskyKeyInt daemon).
 *
 * Used for HMI testing: injects exactly the same keypad messages the
 * daemon would send, without touching physical keys. Handy for waking
 * the screensaver or driving VERB/NOUN action sequences end-to-end.
 *
 * Usage:
 *   keysend [-d ms] <hexKeyId>...   send one or more keystrokes
 *   keysend status                  show queue key/qid and pending message count
 *
 * Examples (keyIds from Event.h):
 *   keysend ffffffff                          wake screensaver (E_NO_EVENT)
 *   keysend -d 400 7fffffff fffffbff bfffffff f7ffffff fffffffdf
 *       VERB 3 NOUN 0 ENTER — disable screensaver
 *
 * Messages are spaced by -d milliseconds (default 400, minimum 250)
 * because dsky processes at most one event per frame; anything else is
 * dropped with a "@TODO Handle more than one event" warning.
 *
 * The queue is identified by ftok("/var/local/dskykeyint", 10) — the same
 * key the app uses (see config.h.in). dsky owns and creates the queue, so
 * it must be running; keysend never creates the queue.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/stat.h>

// Single source of truth for the message layout (shared with app + daemon)
#include "../KeyIntMsgDef.h"

#define KEY_MSG_QUEUE_FILE "/var/local/dskykeyint"
#define KEY_MSG_QUEUE_ID   10

#define DEFAULT_DELAY_MS 400
#define MIN_DELAY_MS     250

/**
 * @brief Resolve the input queue key via ftok.
 * @return The queue key on success.
 * @note Exits with an error message if the marker file is missing or ftok fails.
 */
static long keyQueueKey()
{
    struct stat st;

    if (stat(KEY_MSG_QUEUE_FILE, &st) != 0) {
        fprintf(stderr, "keysend: cannot stat %s: %s\n",
                KEY_MSG_QUEUE_FILE, strerror(errno));
        exit(1);
    }

    long key = ftok(KEY_MSG_QUEUE_FILE, KEY_MSG_QUEUE_ID);
    if (key == -1) {
        fprintf(stderr, "keysend: ftok(%s, %d) failed: %s\n",
                KEY_MSG_QUEUE_FILE, KEY_MSG_QUEUE_ID, strerror(errno));
        exit(1);
    }

    return key;
}

/**
 * @brief Open the existing input queue (never creates it).
 * @param key Queue key from keyQueueKey().
 * @return Message queue id on success.
 * @note Exits with an error message if dsky is not running yet.
 */
static int openKeyQueue(long key)
{
    int qid = msgget(key, 0);
    if (qid == -1) {
        fprintf(stderr, "keysend: msgget(0x%lx) failed: %s — is dsky running?\n",
                (unsigned long)key, strerror(errno));
        exit(1);
    }
    return qid;
}

/**
 * @brief Print queue key, id and pending message count.
 * @param key Queue key from keyQueueKey().
 */
static void showStatus(long key)
{
    int qid = openKeyQueue(key);

    struct msqid_ds info;
    if (msgctl(qid, IPC_STAT, &info) != 0) {
        fprintf(stderr, "keysend: msgctl(IPC_STAT) failed: %s\n", strerror(errno));
        exit(1);
    }

    printf("queue file : %s\n", KEY_MSG_QUEUE_FILE);
    printf("queue key  : 0x%lx\n", (unsigned long)key);
    printf("queue id   : %d\n", qid);
    printf("pending msg: %lu\n", (unsigned long)info.msg_qbytes > 0 ? info.msg_qnum : 0UL);
}

int main(int argc, char **argv)
{
    unsigned int delayMs = DEFAULT_DELAY_MS;
    int i = 1;

    // Optional -d <ms> delay between messages
    if (i + 1 < argc && strcmp(argv[i], "-d") == 0) {
        delayMs = (unsigned int)strtoul(argv[i + 1], nullptr, 10);
        if (delayMs < MIN_DELAY_MS) {
            fprintf(stderr, "keysend: clamping delay to minimum %d ms\n", MIN_DELAY_MS);
            delayMs = MIN_DELAY_MS;
        }
        i += 2;
    }

    long key = keyQueueKey();

    if (i < argc && strcmp(argv[i], "status") == 0) {
        showStatus(key);
        return 0;
    }

    if (i >= argc) {
        fprintf(stderr,
                "usage: %s [-d ms] <hexKeyId>...\n"
                "       %s status\n",
                argv[0], argv[0]);
        return 1;
    }

    int qid = openKeyQueue(key);

    for (; i < argc; i++) {
        unsigned int keyId = (unsigned int)strtoul(argv[i], nullptr, 16);

        KeyMsg_t msg;
        msg.mType = 1;
        msg.mData.keyId = keyId;
        msg.mData.keyModifier = KM_SHORT;

        if (msgsnd(qid, &msg, sizeof(KeyMsg_t) - sizeof(long), 0) != 0) {
            fprintf(stderr, "keysend: msgsnd(0x%08x) failed: %s\n",
                    keyId, strerror(errno));
            return 1;
        }

        printf("sent keyId 0x%08x\n", keyId);

        // Space messages so dsky drains exactly one per frame
        if (i + 1 < argc) {
            usleep((useconds_t)(delayMs * 1000));
        }
    }

    return 0;
}
