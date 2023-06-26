#include "DskyException.h"

DskyException::DskyException(std::string msg)
{
    msg_ = msg;
}

const char * DskyException::what() {

    return msg_.c_str();
}