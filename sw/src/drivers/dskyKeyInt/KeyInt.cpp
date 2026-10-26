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
int KeyInt::lastLeft_ = 0xFFFF;
int KeyInt::lastRight_ = 0xFFFF;
std::mutex KeyInt::stateMutex_;

/** @brief Polling fallback period in milliseconds (50 ms). */
static const uint32_t KEY_POLL_INTERVAL_MS = 50;

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

  /* Register the periodic polling fallback. It makes key detection robust
   * against a dead/intermittent INT line and against i2c NACKs that corrupt
   * a read at the exact moment of an edge (a missed poll tick self-corrects
   * on the next one, a missed edge is lost forever). Must be registered
   * after outMsgQueue_ exists. */
  if (gpioSetTimerFunc(0, KEY_POLL_INTERVAL_MS, pollHandler) < 0)
  {
    LogWarning << "Fail registering key polling timer" << std::endl;
  }
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

    try
    {
      // Read data from the PCF8575
      int leftData = leftKbd_->read();
      int rightData = rightKbd_->read();

      processState(leftData, rightData);
    }
    catch (const std::exception &e)
    {
      LogWarning << "Fail accessing to the PCF8575" << std::endl;
    }
  }
}

void KeyInt::pollHandler(void)
{
  /* Polling fallback: catches key changes even when the INT line is open or
   * intermittent. Transient i2c failures (e.g. NACKs under LED load) are
   * skipped silently — the next tick re-reads and the state comparison
   * self-corrects, so a held key is still detected. */
  try
  {
    int leftData = leftKbd_->read();
    int rightData = rightKbd_->read();

    processState(leftData, rightData);
  }
  catch (const std::exception &e)
  {
    /* Transient i2c error: skip this tick */
  }
}

void KeyInt::processState(int leftData, int rightData)
{
  // Serialize the dedup check and the emit path (INT alert thread and poll
  // timer thread both reach here)
  std::lock_guard<std::mutex> lock(stateMutex_);

  // Only emit on a state change (dedup between INT edge and poll ticks)
  if ((leftData == lastLeft_) && (rightData == lastRight_))
  {
    return;
  }

  lastLeft_ = leftData;
  lastRight_ = rightData;

  // Only process data when data is available
  if ((leftData != 0xFFFF) || (rightData != 0xFFFF))
  {

    /* Compose message to send */
    KeyMsg_t msg;

    msg.mType = 1;
    /* Cast to unsigned before shifting: a signed shift into the sign bit
     * is undefined behavior (leftData can have bit 15 set). */
    msg.mData.keyId = static_cast<unsigned int>(leftData) << 16 |
                      static_cast<unsigned int>(rightData);
    msg.mData.keyModifier = KM_SHORT;

    std::cout << "KeyId: 0x" << std::hex << msg.mData.keyId << std::endl;

    /* Queue message for sending */
    outMsgQueue_->send(msg);
  }
}
