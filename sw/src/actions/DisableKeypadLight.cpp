#include "DisableKeypadLight.h"

// Configuration
#include "config.h"

// Message Queue
#include "MsgQueue.h"
#include "KeyIntMsgDef.h"

// Logging
#include "Logger.h"

DisableKeypadLight::DisableKeypadLight(HmiData &data) : Action(2, 0, data, ACTION_SINGLE)
{
}

void DisableKeypadLight::operation() 
{
    /* Message queue object */
    MsgQueue msgQueue = MsgQueue(CMD_MSG_QUEUE_FILE,
                               CMD_MSG_QUEUE_ID, MSG_QUEUE_OUT);

    /* Command to transmit */
    CommMsg_t cmd;

    cmd.mType = 1;
    cmd.cmd.keylight.enable = false;
    cmd.cmd.keylight.brightness = 0;

    LogTrace << "Running Keypad Light OFF" << std::endl;

    msgQueue.send(cmd);
}
