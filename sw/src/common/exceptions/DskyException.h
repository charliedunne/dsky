#ifndef _DSKYEXCEPTION_H_
#define _DSKYEXCEPTION_H_

// Logging
#include "Logger.h"

// STL
#include <string>

class DskyException : public std::exception
{
protected:
    std::string msg_;

public:
    DskyException(std::string msg);

    const char * what();
};

#endif /* _DSKYEXCEPTION_H_ */