#include "Keyboard.h"

/* Logging */
#include <Logger.h>

#include <sys/mman.h>
#include <fcntl.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>

Keyboard::Keyboard(char *file)
{

    fd_ = -1;

    if ((fd_ = open(file, O_RDWR, 0)) == -1)
    {

        LogError << "Error in Keyboard Creation, file (" << file << ")" << std::endl;
    }

    // Open the file in shared memory
    shared_ = (int *)mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
}

Keyboard::~Keyboard()
{
}

void Keyboard::push(Event e)
{
    events_.push_back(e);
}

Event Keyboard::pop()
{

    Event e = events_.back();
    events_.pop_back();

    return e;
}

void Keyboard::update()
{

    /**
     * @todo Only one single envent in the queue
     * for the moment
     */

    // Copy the current data of the shared memory
    Event e((EventId_t)(*shared_));

    if (e.getEvent() != E_NO_EVENT)
    {
        push(e);

        // Clean share memory
        *shared_ = (int)E_NO_EVENT;
    }
}

bool Keyboard::eventsAvailable()
{

    if (events_.size() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}