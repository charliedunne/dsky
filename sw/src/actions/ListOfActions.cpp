#include "ListOfActions.h"

// STL
#include <stdexcept>

ListOfActions::ListOfActions() {}
ListOfActions::~ListOfActions() {}

void ListOfActions::registerAction(Action * action) {
    actions_.push_back(action);
}

Action * ListOfActions::operator()(HmiData &data)
{
    int verb = data.getRightLcdData().nVerb;
    int noun = data.getRightLcdData().nNoun;

    /* Find the Action to do and execute */
    for (int i = 0; i < actions_.size(); ++i)
    {
        if ((actions_[i]->getVerb() == verb) &&
            (actions_[i]->getNoun() == noun))
        {
            return actions_[i];        
        }
    }

    /* In case of not find the verb required rise an exception */
    throw std::domain_error("Unregistered Verb");
}

bool ListOfActions::needNoun(int verb) {

    for (int i = 0; i < actions_.size(); ++i)
    {
        if (actions_[i]->getVerb() == verb)
        {
            if (actions_[i]->getNoun() == -1) {
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
    //throw std::domain_error("Unregistered Verb");
    return true;
}