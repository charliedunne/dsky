#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <string>

class Configuration
{

private:
    std::string file_;


public:
    Configuration(std::string f);
    virtual ~Configuration();
};

#endif /* _CONFIGURATION_H_ */