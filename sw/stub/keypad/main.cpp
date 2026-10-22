#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>

// Message queues
#include <sys/ipc.h>
#include <sys/msg.h>

// Signal handling
#include <signal.h>

// Message definition
#include "KeyIntMsgDef.h"
#include "Event.h"

// volatile sig_atomic_t is the only type guaranteed safe to set in a signal
// handler and read elsewhere. We must not call iostream from here.
volatile sig_atomic_t running = 1;

void handler(int s) {
    (void)s;
    running = 0;
}

int main()
{
    KeyMsg_t msg;

    // Capture SIGINT so the stub stops gracefully instead of being killed
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    // Generate Unique Message queue key
    key_t key_ = ftok("/var/local/dskykeyint", 10);
    if (key_ == -1)
    {
        std::cerr << "Error: message queue marker file '/var/local/dskykeyint' does not exist."
                  << " Create it with: sudo touch /var/local/dskykeyint" << std::endl;
        return 1;
    }

    // Create the message queue
    int msgId_ = msgget(key_, 0666 | IPC_CREAT);
    if (msgId_ == -1)
    {
        throw std::domain_error("msgget() Error");
    }

    std::cout << "Keypad stub ready. Send keys: 1-9, 0, v(noun), n(noun), e(enter), r(reset),"
              << " c(clear), p(program), k(key release), +, -" << std::endl;
    std::cout << "Press Ctrl+C to quit." << std::endl;

    char press = 0;

    while (running)
    {
        // Read one key command from stdin
        if (!(std::cin >> press))
        {
            // EOF or read error
            break;
        }

        // Re-check after the blocking read so Ctrl+C stops us at the next point
        if (!running)
        {
            break;
        }

        switch (press)
        {

        case '1':
            std::cout << "E_KEY_1" << std::endl;
            msg.mData.keyId = E_KEY_1;
            break;

        case '2':
            std::cout << "E_KEY_2" << std::endl;
            msg.mData.keyId = E_KEY_2;
            break;

        case '3':
            std::cout << "E_KEY_3" << std::endl;
            msg.mData.keyId = E_KEY_3;
            break;
        case '4':
            std::cout << "E_KEY_4" << std::endl;
            msg.mData.keyId = E_KEY_4;
            break;
        case '5':
            std::cout << "E_KEY_5" << std::endl;
            msg.mData.keyId = E_KEY_5;
            break;
        case '6':
            std::cout << "E_KEY_6" << std::endl;
            msg.mData.keyId = E_KEY_6;
            break;
        case '7':
            std::cout << "E_KEY_7" << std::endl;
            msg.mData.keyId = E_KEY_7;
            break;
        case '8':
            std::cout << "E_KEY_8" << std::endl;
            msg.mData.keyId = E_KEY_8;
            break;
        case '9':
            std::cout << "E_KEY_9" << std::endl;
            msg.mData.keyId = E_KEY_9;
            break;
        case '0':
            std::cout << "E_KEY_0" << std::endl;
            msg.mData.keyId = E_KEY_0;
            break;

        case 'v':
            std::cout << "E_KEY_VERB" << std::endl;
            msg.mData.keyId = E_KEY_VERB;
            break;

        case 'n':
            std::cout << "E_KEY_NOUN" << std::endl;
            msg.mData.keyId = E_KEY_NOUN;
            break;

        case 'e':
            std::cout << "E_KEY_ENTR" << std::endl;
            msg.mData.keyId = E_KEY_ENTR;
            break;

        case 'r':
            std::cout << "E_KEY_RSET" << std::endl;
            msg.mData.keyId = E_KEY_RSET;
            break;

        case 'c':
            std::cout << "E_KEY_CLR" << std::endl;
            msg.mData.keyId = E_KEY_CLR;
            break;

        case 'p':
            std::cout << "E_KEY_PRO" << std::endl;
            msg.mData.keyId = E_KEY_PRO;
            break;

        case 'k':
            std::cout << "E_KEY_KEYREL" << std::endl;
            msg.mData.keyId = E_KEY_KEYREL;
            break;

        case '+':
            std::cout << "E_KEY_PLUS" << std::endl;
            msg.mData.keyId = E_KEY_PLUS;
            break;

        case '-':
            std::cout << "E_KEY_MINUS" << std::endl;
            msg.mData.keyId = E_KEY_MINUS;
            break;


        default:
            // Unknown command: ignore and keep running
            std::cout << "Unknown key '" << press << "', ignored." << std::endl;
            continue;
        }

        msg.mData.keyModifier = KM_SHORT;
        msg.mType = 1;

        if (msgsnd(msgId_, &msg, sizeof(KeyMsg_t), 0) != 0)
        {
            std::cout << "Sending key ERROR!" << std::endl;
        }
    }

    // Leave the queue in place: dsky polls it with IPC_NOWAIT and recreates it
    // if needed. Removing it here could race with a reader mid-poll.
    std::cout << "Keypad stub shutting down cleanly." << std::endl;

    return 0;
}
