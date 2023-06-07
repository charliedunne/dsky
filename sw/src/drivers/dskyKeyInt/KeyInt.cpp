// Provided interface
#include "KeyInt.h"

// STL
#include <stdexcept>
#include <iostream>

// Configuration
#include "config.h"

// Logging
#include "Logger.h"

/* Definition of static members */
int KeyInt::gpioInt_ = 0;
Pcf8575 *KeyInt::leftKbd_ = nullptr;
Pcf8575 *KeyInt::rightKbd_ = nullptr;
MsgQueue *KeyInt::outMsgQueue_ = nullptr;

KeyInt::KeyInt(const int iicBus,
               const int iicAddrLeft, const int iicAddrRigth,
               const int gpioInt)
{
  // Save info locally
  gpioInt_ = gpioInt;

  // Initialize the PCF8575 controller
  leftKbd_ = new Pcf8575(iicBus, iicAddrLeft);
  rightKbd_ = new Pcf8575(iicBus, iicAddrRigth);

  // Set the interruption GPIO configuration
  gpioSetMode(gpioInt_, PI_INPUT);

  outMsgQueue_ = new MsgQueue(KEY_INT_MSG_QUEUE_FILE,
                              KEY_INT_MSG_QUEUE_ID, MSG_QUEUE_OUT);

  // Register callback for interruption
  gpioSetAlertFunc(gpioInt_, intHandler);
}

KeyInt::~KeyInt()
{
  // Remove message queue
  delete outMsgQueue_;
}

void KeyInt::intHandler(int gpio, int level, uint32_t tick)
{
  // Read the interruption line
  int intLine = gpioRead(gpioInt_);

  if (intLine == 0)
  {

    // Read data from the PCF8575
    int leftData = leftKbd_->read();
    int rightData = rightKbd_->read();

    // Only process data when data is available
    if ((leftData != 0xFFFF) || (rightData != 0xFFFF))
    {

      /* Compose message to send */
      KeyMsg_t msg;

      msg.mType = 1;
      msg.mData.keyId = leftData << 16 | rightData;
      msg.mData.keyModifier = KM_SHORT;

      std::cout << "KeyId: 0x" << std::hex << msg.mData.keyId << std::endl;

      /* Queue message for sending */
      outMsgQueue_->send(msg);
    }
  }
}
