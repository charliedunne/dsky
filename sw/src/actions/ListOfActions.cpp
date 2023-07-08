#include "ListOfActions.h"

// STL
#include <stdexcept>

// Exceptions
#include "ExcInvalidCommand.h"

ListOfActions::ListOfActions() {}
ListOfActions::~ListOfActions() {}

void ListOfActions::registerAction(Action &action) {
    actions_.push_back(action);
}

Action & ListOfActions::operator()(HmiData &data)
{
    int verb = data.getRightLcdData().nVerb;
    int noun = data.getRightLcdData().nNoun;

    /* Find the Action to do and execute */
    for (int i = 0; i < actions_.size(); ++i)
    {
        if ((actions_[i].operator Action &().getVerb() == verb) &&
            (actions_[i].operator Action &().getNoun() == noun))
        {
            return actions_[i];        
        }
    }

    /* In case of not find the verb required rise an exception */
    //throw std::domain_error("Unregistered Verb");
    throw ExcInvalidCommand(verb, -1);
}

bool ListOfActions::needNoun(int verb) {

    for (int i = 0; i < actions_.size(); ++i)
    {
        if (actions_[i].operator Action &().getVerb() == verb)
        {
            if (actions_[i].operator Action &().getNoun() == -1) {
                return false;
            }
            else
            {
                return true;
            }
            break;
        }
    }

    /* In case of not find the verb required rise an exception */
    /** @todo I think it is not necessary to rise an execption here, 
     * just return true so the operation can continue. At least for debuggin purposes */
    throw ExcInvalidCommand(verb, -1);
    return true;
}