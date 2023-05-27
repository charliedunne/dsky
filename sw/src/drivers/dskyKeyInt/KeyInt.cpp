// Provided interface
#include "KeyInt.h"

// Message queues
#include <sys/ipc.h>
#include <sys/msg.h>

// PiGpio Lib
#include "pigpio.h"


static int msgid;


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

    // Register callback for interruption
    gpioSetAlertFunc(gpioInt_, KeyInt::intHandler)
}

KeyInt::~KeyInt()
{
    gpioTerminate();
}

void KeyInt::intHandler()
{
    // Read the interruption line
    int intLine = gpioRead(gpioInt_);

    // Read data from the PCF8575
    int leftData = leftKbd_->read(); 
    int rightData = rightKbd_->read(); 

    // Only process data when data is available
    if ((lefData != 0xFFFF) || (rightData != 0xFFFF)) {

        /** @todo Compose the output word and add to the message queue */
    }

}