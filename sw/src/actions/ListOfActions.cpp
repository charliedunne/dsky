#include "ListOfActions.h"

// STL
#include <stdexcept>

ListOfActions::ListOfActions() {}
ListOfActions::~ListOfActions() {}

void ListOfActions::registerAction(Action * action) {
    actions_.push_back(action);
}

Action * ListOfActions::operator()(int verb, int noun)
{
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
    throw std::domain_error("Unregistered Verb");
}