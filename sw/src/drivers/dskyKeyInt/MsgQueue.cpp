#include "MsgQueue.h"

// STL
#include <stdexcept>
#include <vector>
#include <typeinfo>

MsgQueue::MsgQueue(char *queueFile, int queueId, MsgQueueDirection_e dir)
{
    dir_ = dir;

    // Generate Unique Message queue key
    key_ = ftok(queueFile, queueId);
    if (key_ == -1)
    {
        throw std::domain_error("ftok() Error");
    }

    // Create the message queue
    msgId_ = msgget(key_, 0666 | IPC_CREAT);
    if (msgId_ == -1)
    {
        throw std::domain_error("msgget() Error");
    }
}

MsgQueue::~MsgQueue()
{
    // Remove message queue
    msgctl(msgId_, IPC_RMID, NULL);
}



