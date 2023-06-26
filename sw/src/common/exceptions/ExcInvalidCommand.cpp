#include "ExcInvalidCommand.h"

ExcInvalidCommand::ExcInvalidCommand(const int verb, const int noun) : 
DskyException("Invalid command. ")
{
    msg_ = msg_ + " (V" + std::to_string(verb) + "N" + std::to_string(noun) + ")";
}

ExcInvalidCommand::~ExcInvalidCommand()
{
}
