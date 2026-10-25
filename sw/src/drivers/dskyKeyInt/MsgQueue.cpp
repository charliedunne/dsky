#include "MsgQueue.h"

// STL
#include <stdexcept>
#include <iostream>

// System
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

// Common types
#include "KeyIntMsgDef.h"

MsgQueue::MsgQueue(std::string queueFile, int queueId, MsgQueueDirection_e dir)
{
    dir_ = dir;

    // Check that the message queue marker file exists before calling ftok(),
    // which requires an existing file to derive the IPC key from.
    struct stat st;
    if (stat(queueFile.c_str(), &st) != 0) {
        std::cerr << "Error: message queue marker file '" << queueFile
                  << "' does not exist. Create it with:"
                  << std::endl
                  << "    sudo touch " << queueFile << std::endl;
        throw std::domain_error("ftok() Error");
    }

    // Generate Unique Message queue key
    key_ = ftok(queueFile.c_str(), queueId);
    if (key_ == -1)
    {
        std::cerr << "Error: ftok() failed for '" << queueFile
                  << "' (" << strerror(errno) << ")" << std::endl;
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
    /* Message queues are system resources shared across processes.
     * Do NOT remove them here — doing so would destroy the queue
     * for all other users (e.g. the daemon blocking in msgrcv).
     * Queues persist until explicitly removed or system reboot. */
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
        if (msgrcv(msgId_, &msg, sizeof(T), 0, 0) < 0)
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
        if (msgrcv(msgId_, &msg, sizeof(T), 0, IPC_NOWAIT) < 0)
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
