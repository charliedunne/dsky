#include "CmdManager.h"

// Configuration
#include "config.h"

CmdManager::CmdManager()
{
    inMsgQueue_ = new MsgQueue(CMD_MSG_QUEUE_FILE,
                               CMD_MSG_QUEUE_ID, MSG_QUEUE_IN);
}

CmdManager::~CmdManager() {}

void CmdManager::waitForCommand()
{
    /* Blocking call */
    inMsgQueue_->recv(cmd_);
}

CommMsg_t &CmdManager::getCommand()
{
    return cmd_;
}

