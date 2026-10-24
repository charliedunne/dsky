#include "ListOfActions.h"

// STL
#include <stdexcept>
#include <algorithm>   // std::sort
#include <cstdio>      // std::snprintf

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

std::vector<std::string> ListOfActions::describe() const {

    // Build a sortable list of pointers to the registered actions
    std::vector<const Action*> sorted;
    for (const auto &it : actions_) {
        sorted.push_back(&it.get());
    }

    // Sort by (verb, noun) ascending so the menu is deterministic
    std::sort(sorted.begin(), sorted.end(),
              [](const Action *a, const Action *b) {
                  if (a->getVerb() != b->getVerb())
                      return a->getVerb() < b->getVerb();
                  return a->getNoun() < b->getNoun();
              });

    // Format each entry as "Vxx LABEL" or "Vxx Nyy LABEL"
    std::vector<std::string> out;
    for (const Action *a : sorted) {
        char buf[64];
        if (a->getNoun() == -1) {
            std::snprintf(buf, sizeof(buf), "V%02d %s", a->getVerb(), a->getLabel().c_str());
        } else {
            std::snprintf(buf, sizeof(buf), "V%02d N%02d %s", a->getVerb(), a->getNoun(), a->getLabel().c_str());
        }
        out.push_back(std::string(buf));
    }

    return out;
}