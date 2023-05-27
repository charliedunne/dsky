// Provided interface
#include "Pcf8575.h"

// STL
#include <stdexcept>

// pigpio
#include "pigpio.h"

int Pcf8575::read()
{

    // Output data
    int data = 0;

    // Handle for the i2c operations
    int handle = -1;

    // Open the i2c communication
    handle = i2cOpen(iicBus_, iicAddr_, 0);

    if (handle < 0)
    {

        throw domain_error("i2cOpen() Error")
    }

    // Read all GPIO data
    data = i2cReadWordData(handle, 1);

    if (data < 0)
    {

        throw domain_error("i2cReadWordData() Error")
    }

    // Close i2c communication
    if (i2cClose(handle) != 0)
    {

        throw domain_error("i2cClose() Error")
    }

    return data;
}