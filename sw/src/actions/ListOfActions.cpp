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

bool ListOfActions::needValue(int verb, int noun) {

    /* Only the write-variable verb prompts for a value */
    if (verb != WRITE_VAR_VERB) {
        return false;
    }

    /* A value is needed only when an action is registered
     * for this exact (verb, noun) pair */
    for (int i = 0; i < actions_.size(); ++i)
    {
        if ((actions_[i].operator Action &().getVerb() == verb) &&
            (actions_[i].operator Action &().getNoun() == noun))
        {
            return true;
        }
    }

    /* Unregistered noun: no value prompt, the command will be
     * rejected later with ExcInvalidCommand in run() */
    return false;
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

    // Format entries as "Vxx LABEL" or "Vxx Nyy LABEL".
    // A verb whose nouns all share the same label is collapsed into a
    // single "Vxx NXX LABEL" line (e.g. READ_VAR / WRITE_VAR).
    std::vector<std::string> out;
    for (size_t i = 0; i < sorted.size(); ) {
        int verb = sorted[i]->getVerb();

        // Collect the whole group of entries sharing this verb
        size_t j = i + 1;
        while (j < sorted.size() && sorted[j]->getVerb() == verb)
            ++j;

        // Collapsible only if the group has more than one entry, every
        // entry has a noun, and all labels are identical.
        bool collapse = (j > i + 1);
        for (size_t k = i + 1; k < j && collapse; ++k) {
            if (sorted[i]->getNoun() == -1 || sorted[k]->getNoun() == -1 ||
                sorted[k]->getLabel() != sorted[i]->getLabel())
                collapse = false;
        }

        char buf[64];
        if (collapse) {
            std::snprintf(buf, sizeof(buf), "V%02d NXX %s", verb, sorted[i]->getLabel().c_str());
            out.push_back(std::string(buf));
        } else {
            for (size_t k = i; k < j; ++k) {
                if (sorted[k]->getNoun() == -1) {
                    std::snprintf(buf, sizeof(buf), "V%02d %s", verb, sorted[k]->getLabel().c_str());
                } else {
                    std::snprintf(buf, sizeof(buf), "V%02d N%02d %s", verb, sorted[k]->getNoun(), sorted[k]->getLabel().c_str());
                }
                out.push_back(std::string(buf));
            }
        }

        i = j;
    }

    return out;
}

void ListOfActions::stopAll() {
    for (const auto &it : actions_) {
        it.get().stop();
    }
}