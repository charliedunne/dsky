#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

/* Local interface */
#include "Event.h"

/* STD Library */
#include <vector>

class Keyboard
{

private:

    /**
     * @brief File descriptor for the mmap
     * 
     */
    int fd_;

    /**
     * @brief Pointer to the shared memory
     * 
     */
    int * shared_;

    /**
     * @brief Events queue
     * 
     */
    std::vector<Event> events_;

    /**
     * @brief Add a new event to the queue
     * 
     * @param e 
     */
    void push(Event e);

    /**
     * @brief Get event from queue
     * 
     * @return Event 
     */
    Event pop();    

public:
    Keyboard(char * file);
    virtual ~Keyboard();

    /**
     * @brief Operation to run every frame. It will check
     * the shared memory looking for keyboard events.
     * If any is found it will be stored in the internal
     * vector @b events_
     * 
     */
    void update();

    /**
     * @brief 
     * 
     * @return true if there are 1 or more events in the queue
     * @return false if there are no any event available
     */
    bool eventsAvailable();

};

#endif /* _KEYBOARD_H_ */
