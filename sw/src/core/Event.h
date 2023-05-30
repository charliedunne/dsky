#ifndef _EVENT_H_
#define _EVENT_H_

#include <ctime>

typedef enum {
    E_KEY_0 = 0xf7ffffff,
    E_KEY_1 = 0xffefffff,
    E_KEY_2 = 0xffffdfff,
    E_KEY_3 = 0xfffffbff,
    E_KEY_4 = 0xfff7ffff,
    E_KEY_5 = 0xffffbfff,
    E_KEY_6 = 0xfffff7ff,
    E_KEY_7 = 0xfffbffff,
    E_KEY_8 = 0xffff7fff,
    E_KEY_9 = 0xffffefff,
    E_KEY_PLUS = 0xdfffffff,
    E_KEY_MINUS = 0xefffffff,
    E_KEY_VERB = 0x7fffffff,
    E_KEY_NOUN = 0xbfffffff,
    E_KEY_CLR = 0xfffffffb,
    E_KEY_KEYREL = 0xffffffef,
    E_KEY_ENTR = 0xffffffdf,
    E_KEY_RSET = 0xffffffbf,
    E_KEY_PRO = 0xfffffff7,
    E_NO_EVENT = 0xffffffff
} EventId_t;

class Event {

private:
    EventId_t eventId_;
    std::time_t timestamp_;

public:
    Event();
    Event(const EventId_t event);
    virtual ~Event();

    /**
     * @brief Record the Event
     * 
     * @param event 
     */
    void record(const EventId_t event);

    /**
     * @brief Get the Event object
     * 
     * @return EventId_t 
     */
    EventId_t getEvent();

    /**
     * @brief Get the Timestamp object
     * 
     * @return std::time_t 
     */
    std::time_t getTimestamp();
};

#endif /* _EVENT_H_ */
