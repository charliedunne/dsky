#ifndef _EVENT_H_
#define _EVENT_H_

#include <ctime>

typedef enum {
    E_KEY_0 = 0,
    E_KEY_1 = 1,
    E_KEY_2 = 2,
    E_KEY_3 = 3,
    E_KEY_4 = 4,
    E_KEY_5 = 5,
    E_KEY_6 = 6,
    E_KEY_7 = 7,
    E_KEY_8 = 8,
    E_KEY_9 = 9,
    E_KEY_PLUS,
    E_KEY_MINUS,
    E_KEY_VERB,
    E_KEY_NOUN,
    E_KEY_CLR,
    E_KEY_KEYREL,
    E_KEY_ENTR,
    E_KEY_RSET,
    E_KEY_PRO,
    E_NO_EVENT = 0xFFFF
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