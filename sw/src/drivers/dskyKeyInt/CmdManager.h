#ifndef _CMDMANAGER_H_
#define _CMDMANAGER_H_

#include "KeyIntMsgDef.h"

// Message queue
#include "MsgQueue.h"

// STL
#include <vector>

class CmdManager
{
private:
    /**
     * @brief Buffer with the command received
     */
    CommMsg_t cmd_;

    /**
     * @brief Input Command queue for the output events
     */
    MsgQueue *inMsgQueue_;

public:
    CmdManager();
    virtual ~CmdManager();

    /**
     * @brief Blocking call for waiting for commands in the message queue
     *
     */
    void waitForCommand();

    /**
     * @brief Get the Commands object
     *
     * @return CommMsg_t Command
     */
    CommMsg_t &getCommand();
};

#endif /* _CMDMANAGER_H_ */