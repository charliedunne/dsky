#ifndef _EXCINVALIDCOMMAND_H_
#define _EXCINVALIDCOMMAND_H_

// General exceptions
#include "DskyException.h"

class ExcInvalidCommand : public DskyException
{
private:
    int verb_;
    int noun_;

public:
    ExcInvalidCommand(const int verb, const int noun);
    virtual ~ExcInvalidCommand();
};

#endif /* _EXCINVALIDCOMMAND_H_ */