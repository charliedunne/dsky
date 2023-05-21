// Provided interface
#include "Pcf8575.h"

// pigpio
#include "pigpio.h"


int Pcf8575::read() {

    // Output data
    int data = 0;

    // Handle for the i2c operations
    int handle = -1;

    // Open the i2c communication
    handle = i2cOpen(iicBus_, iicAddr_, 0);
    
    if (handle < 0) {

        /** @todo handle error */
    }

    // Read all GPIO data
    data = i2cReadWordData(handle, 1);

    if (out < 0) {

        /** @todo handle error */
    }

    // Close i2c communication
    if (i2cClose(handle) != 0) {
        
        /** @todo handle error */
    }

    return data;
}