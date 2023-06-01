#ifndef _STATUSMACHINE_H_
#define _STATUSMACHINE_H_

#include <string>

typedef enum
{

    MODE_IDLE,
    MODE_V_INPUT,
    MODE_N_INPUT,
    MODE_RUN,
    MODE_ERROR,

} Mode_e;

class StatusMachine
{

private:
    Mode_e mode_;

public:
    StatusMachine();
    virtual ~StatusMachine();

    void transit(Mode_e dstMode);

    Mode_e operator()() const;

    std::string to_string();
};

#endif /* _STATUSMACHINE_H_ */