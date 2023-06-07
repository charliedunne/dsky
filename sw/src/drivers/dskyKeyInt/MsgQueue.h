#ifndef _MSGQUEUE_H_
#define _MSGQUEUE_H_

/* Message queues */
#include <sys/ipc.h>
#include <sys/msg.h>

// STL
#include <stdexcept>

typedef enum
{
    MSG_QUEUE_IN,
    MSG_QUEUE_OUT
} MsgQueueDirection_e;

class MsgQueue
{

private:
    /**
     * @brief Message queue key
     */
    key_t key_;

    /**
     * @brief Message queue id
     *
     */
    int msgId_;

    /**
     * @brief Direction of the msg queue
     */
    MsgQueueDirection_e dir_;

public:
    MsgQueue(char *, int, MsgQueueDirection_e);
    virtual ~MsgQueue();

    template <class T>
    void send(T &msg)
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
    void recv(T &msg)
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
    void poll(T &msg)
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
};

#endif /* _MSGQUEUE_H_ */