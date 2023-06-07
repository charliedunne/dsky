#include "MsgQueue.h"

// STL
#include <stdexcept>

// Common types
#include "KeyIntMsgDef.h"

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

template <class T>
void MsgQueue::send(T &msg)
{

    
    if (dir_ == MSG_QUEUE_OUT)
    {
        if (msgsnd(msgId_, &msg, sizeof(T), 0) != 0)
        {
            throw std::domain_error("msgsnd() Error");
        }
    }
    else
    {
        throw std::domain_error("Send is only compatible with MSG_QUEUE_OUT");
    }
}

template <class T>
void MsgQueue::recv(T &msg)
{
    if (dir_ == MSG_QUEUE_IN)
    {
        /* Vector to return */
        if (msgrcv(msgId_, &msg, sizeof(T), 1, 0) < 0)
        {
            throw std::domain_error("Error in msgrcv (poll)");
        }
    }
    else
    {
        throw std::domain_error("Recv is only compatible with MSG_QUEUE_IN");
    }
}

template <class T>
void MsgQueue::poll(T &msg)
{
    if (dir_ == MSG_QUEUE_IN)
    {
        /* Vector to return */
        if (msgrcv(msgId_, &msg, sizeof(T), 1, IPC_NOWAIT) < 0)
        {
            throw std::domain_error("Error in msgrcv (poll)");
        }
    }
    else
    {
        throw std::domain_error("Poll is only compatible with MSG_QUEUE_IN");
    }
}

template void MsgQueue::send<KeyMsg_t>(KeyMsg_t &);
template void MsgQueue::send<CommMsg_t>(CommMsg_t &);

template void MsgQueue::recv<KeyMsg_t>(KeyMsg_t &);
template void MsgQueue::recv<CommMsg_t>(CommMsg_t &);

template void MsgQueue::poll<KeyMsg_t>(KeyMsg_t &);
template void MsgQueue::poll<CommMsg_t>(CommMsg_t &);
