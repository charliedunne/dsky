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
    void send(T &msg);

    template <class T>
    void recv(T &msg);

    template <class T>
    void poll(T &msg);
};

#endif /* _MSGQUEUE_H_ */