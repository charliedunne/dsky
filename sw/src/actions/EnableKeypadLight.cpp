#include "EnableKeypadLight.h"

// Configuration
#include "config.h"

// Message Queue
#include "MsgQueue.h"
#include "KeyIntMsgDef.h"

// Logging
#include "Logger.h"

EnableKeypadLight::EnableKeypadLight(HmiData &data) : Action(2, 1, data, ACTION_SINGLE)
{
}

void EnableKeypadLight::operation() 
{
    /* Message queue object */
    MsgQueue msgQueue = MsgQueue(CMD_MSG_QUEUE_FILE,
                               CMD_MSG_QUEUE_ID, MSG_QUEUE_OUT);

    /* Command to transmit */
    CommMsg_t cmd;

    cmd.mType = 1;
    cmd.cmd.keylight.enable = true;
    cmd.cmd.keylight.brightness = 255;

    LogTrace << "Running Keypad Light ON" << std::endl;

    msgQueue.send(cmd);
}
