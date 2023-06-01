#include "ListOfActions.h"

ListOfActions::ListOfActions() {}
ListOfActions::~ListOfActions() {}

void ListOfActions::registerAction(Action * action) {
    actions_.push_back(action);
}

void ListOfActions::operator()(int verb, int noun)
{
    /* Find the Action to do and execute */
    for (int i = 0; i < actions_.size(); ++i)
    {
        if ((actions_[i]->getVerb() == verb) &&
            (actions_[i]->getNoun() == noun))
        {
            actions_[i]->run();
            break;
        }
    }
}