// Provided interface
#include "KeyInt.h"

// STL
#include <stdexcept>
#include <iostream>

// Configuration
#include "config.h"

// Message queues
#include <sys/ipc.h>
#include <sys/msg.h>

// Logging
#include "Logger.h"

/* Definition of static members */
int KeyInt::gpioInt_ = 0;
Pcf8575 * KeyInt::leftKbd_ = NULL;
Pcf8575 * KeyInt::rightKbd_ = NULL;
key_t KeyInt::key_;
int KeyInt::msgId_ = 0;

KeyInt::KeyInt(const int iicBus,
               const int iicAddrLeft, const int iicAddrRigth,
               const int gpioInt)
{
    // Save info locally
    gpioInt_ = gpioInt;

    // Initialize the PCF8575 controller
    leftKbd_ = new Pcf8575(iicBus, iicAddrLeft);
    rightKbd_ = new Pcf8575(iicBus, iicAddrRigth);

    // Initialize pigpio driver
    gpioInitialise();

    // Set the interruption GPIO configuration
    gpioSetMode(gpioInt_, PI_INPUT);

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

    // Register callback for interruption
    gpioSetAlertFunc(gpioInt_, KeyInt::intHandler);
}

KeyInt::~KeyInt()
{
    gpioTerminate();
}

void KeyInt::intHandler(int gpio, int level, uint32_t tick)
{
    // Read the interruption line
    int intLine = gpioRead(gpioInt_);

    // Read data from the PCF8575
    int leftData = leftKbd_->read(); 
    int rightData = rightKbd_->read(); 

    // Only process data when data is available
    if ((leftData != 0xFFFF) || (rightData != 0xFFFF)) {

        /* Compose message to send */
        KeyMsg_t msg;

        msg.mType = 1;
        msg.mData.keyId = leftData << 16 | rightData;
        msg.mData.keyModifier = KM_SHORT;

        LogTrace << "KeyId: 0x" << std::hex << msg.mData.keyId << std::endl;

        /* Queue message for sending */
        if (msgsnd(msgId_, &msg, sizeof(KeyMsg_t), 0) != 0)
        {
            throw std::domain_error("msgsnd() Error");
        }
    }

}