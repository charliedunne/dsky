#ifndef _THREAD_H_
#define _THREAD_H_

#include <thread>

class Thread
{

private:
    
    /* Pointer to the thread */
    std::thread *thread_;

    /* Execution flag */
    bool running_;

    virtual void run() = 0;

}

#endif /* _THREAD_H_ */