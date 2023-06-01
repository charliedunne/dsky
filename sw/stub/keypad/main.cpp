#include <iostream>
#include <string>

// Message queues
#include <sys/ipc.h>
#include <sys/msg.h>

// Message definition
#include "KeyIntMsgDef.h"
#include "Event.h"

int main()
{
    KeyMsg_t msg;

    // Generate Unique Message queue key
    key_t key_ = ftok("/var/local/dskykeyint", 10);
    if (key_ == -1)
    {
        throw std::domain_error("ftok() Error");
    }

    // Create the message queue
    int msgId_ = msgget(key_, 0666 | IPC_CREAT);
    if (msgId_ == -1)
    {
        throw std::domain_error("msgget() Error");
    }

    char press = 0;

    while (press != 7)
    {
        std::cin >> press;

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
            // Remove message queue
            std::cout << "Quit app" << std::endl;
            msgctl(msgId_, IPC_RMID, NULL);
            
            return 0;
        }

        msg.mData.keyModifier = KM_SHORT;
        msg.mType = 1;

        if (msgsnd(msgId_, &msg, sizeof(KeyMsg_t), 0) != 0)
        {
            std::cout << "Sending key ERROR!" << std::endl;
        }
    }

    return 0;
}