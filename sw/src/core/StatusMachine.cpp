#include "StatusMachine.h"

// STL
#include <stdexcept>

// Loging
#include "Logger.h"

StatusMachine::StatusMachine()
{
    mode_ = MODE_IDLE;
}

StatusMachine::~StatusMachine() {}

Mode_e StatusMachine::operator()() const {
    return mode_;
}

std::string StatusMachine::to_string() 
{
    switch (mode_)
    {
        case MODE_IDLE:
        return std::string("IDLE");
        break;

        case MODE_V_INPUT:
        return std::string("V_INPUT");
        break;

        case MODE_N_INPUT:
        return std::string("N_INPUT");
        break;

        case MODE_RUN:
        return std::string("RUN");
        break;

        case MODE_ERROR:
        return std::string("ERROR");
        break;

        default:
        return std::string("UNKNOWN");
        break;
    }

}

void StatusMachine::transit(Mode_e dstMode)
{
    switch (dstMode)
    {
    case MODE_IDLE:
        switch (mode_)
        {

        case MODE_IDLE:
            /* Do nothing */
            break;

        case MODE_V_INPUT:
        case MODE_N_INPUT:
        case MODE_RUN:
        case MODE_ERROR:
            LogDebug << "Transit to IDLE" << std::endl;
            /* Transit */
            mode_ = MODE_IDLE;
            break;
        }
        break;

    case MODE_V_INPUT:

        switch (mode_)
        {

        case MODE_V_INPUT:
            /* Do nothing */
            break;

        case MODE_IDLE:
        case MODE_N_INPUT:
            LogDebug << "Transit to V_INPUT" << std::endl;
            /* Transit */
            mode_ = MODE_V_INPUT;
            break;

        case MODE_RUN:
            throw std::domain_error("Invalid transition from RUN -> V_INPUT");
            break;
        }

        break;

    case MODE_N_INPUT:

        switch (mode_)
        {

        case MODE_N_INPUT:
            /* Do nothing */
            break;

        case MODE_IDLE:
        case MODE_V_INPUT:
            /* Transit */
            LogDebug << "Transit to N_INPUT" << std::endl;
            mode_ = MODE_N_INPUT;
            break;

        case MODE_RUN:
            throw std::domain_error("Invalid transition from RUN -> N_INPUT");
            break;
        }    
        break;

    case MODE_RUN:

        switch (mode_)
        {

        case MODE_RUN:
            /* Do nothing */
            break;

        case MODE_V_INPUT:
        case MODE_N_INPUT:

            LogDebug << "Transit to RUN" << std::endl;        
            /* Transit */
            mode_ = MODE_RUN;
            break;

        case MODE_IDLE:
            throw std::domain_error("Invalid transition from IDLE -> RUN");
            break;
        }    
        break;

    case MODE_ERROR:

        mode_ = MODE_ERROR;
        break;
        
    default:
        throw std::domain_error("Unknown Destination mode");
        break;
    }
}