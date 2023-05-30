// Provided interface
#include "Keypad.h"

// STL
#include <stdexcept>
#include <iostream>

// Configuration
#include "config.h"

// Message Queue Definition
#include "KeyIntMsgDef.h"

Keypad::Keypad() {

    // Generate Unique Message queue key
    key_ = ftok(KEY_INT_MSG_QUEUE_FILE, KEY_INT_MSG_QUEUE_ID);
    if (key_ == -1) {
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

    msgctl(msgId_, IPC_RMID, NULL);
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
