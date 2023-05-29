#ifndef _KEYPAD_H_
#define _KEYPAD_H_

// STL
#include <vector>

// Message queue
#include <sys/ipc.h>
#include <sys/msg.h>

// Internal libraries
#include "Event.h"

class Keypad
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

public:
    /**
     * @brief Construct a new Keypad object
     *
     */
    Keypad();

    /**
     * @brief Destroy the Keypad object
     *
     */
    virtual ~Keypad();

    /**
     * @brief Find in the message queue if there is any (one or more)
     * key event, and get them
     *
     * @attention This is a non-blocking call, just check the message
     * queue finding for any available data, if there is no any, it
     * just return the control inmediatelly.
     *
     * @return std::vector<Event> list of key events registered
     */
    std::vector<Event> getEvents();

    /**
     * @brief Wait for next event
     *
     * @attention This is a blocking call, it will block the control
     * until a new message in the queue is received.
     *
     * @return Event Event received
     */
    Event waitEvent();
};

#endif /* _KEYPAD_H_ */