// Provided interface
#include "Keypad.h"

// STL
#include <stdexcept>
#include <iostream>

// System
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

// Configuration
#include "config.h"

// Message Queue Definition
#include "KeyIntMsgDef.h"

Keypad::Keypad() {

    // Check that the message queue marker file exists before calling ftok(),
    // which requires an existing file to derive the IPC key from.
    struct stat st;
    if (stat(KEY_INT_MSG_QUEUE_FILE, &st) != 0) {
        std::cerr << "Error: message queue marker file '" << KEY_INT_MSG_QUEUE_FILE
                  << "' does not exist. Create it with:"
                  << std::endl
                  << "    sudo touch " << KEY_INT_MSG_QUEUE_FILE << std::endl;
        throw std::domain_error("ftok() Error");
    }

    // Generate Unique Message queue key
    key_ = ftok(KEY_INT_MSG_QUEUE_FILE, KEY_INT_MSG_QUEUE_ID);
    if (key_ == -1) {
        std::cerr << "Error: ftok() failed for '" << KEY_INT_MSG_QUEUE_FILE
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

Keypad::~Keypad() {
    /* Message queues are system resources shared across processes.
     * Do NOT remove them here — doing so would destroy the queue
     * for all other users (e.g. the daemon sending key events). */
}

std::vector<Event> Keypad::getEvents() {

    /* Vector to return */
    std::vector<Event> events;

    /* Container for the message queue data */
    KeyMsg_t msg;

    while(msgrcv(msgId_, &msg, sizeof(KeyMsg_t), 1, IPC_NOWAIT) > 0) 
    {
        Event e = Event((EventId_t)msg.mData.keyId);

	//        std::cout << "Key Received: " << std::hex << msg.mData.keyId << std::endl;

        events.push_back(e);
    }

    return events;
}

Event Keypad::waitEvent() {

    /* Event to return */
    Event e;

    /* Container for the message queue data */
    KeyMsg_t msg;

    if (msgrcv(msgId_, &msg, sizeof(KeyMsg_t), 1, 0) > 0) 
    {
        e.record((EventId_t)msg.mData.keyId);
    }

    return e;
}
