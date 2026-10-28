#ifndef _LISTOFACTIONS_H_
#define _LISTOFACTIONS_H_

// Generic Action
#include "Action.h"

// STL
#include <vector>
#include <string>

/**
 * @brief Command registry/dispatcher mapping (verb, noun) to a registered Action.
 *
 * @note Intentionally NOT an Action subclass: this container is never executed
 *       itself, it only performs cheap stateless lookups for the HMI loop, so
 *       it has no verb/noun of its own and no thread machinery. To expose the
 *       list as a menu item, use ActionVerbList (the Action bridge).
 */
class ListOfActions {

    private:

    /**
     * @brief Array of actions
     */
    std::vector<std::reference_wrapper<Action>> actions_;

    public:

    ListOfActions();
    virtual ~ListOfActions();

    /**
     * @brief Register a new action in the list
     * 
     * @param action[in] Pointer to the action to register 
     */
    void registerAction(Action &action);

    /**
     * @brief Overload to the operator() to make a functor
     * 
     * @param data[in] Hmi data required for the action
     */
    Action & operator()(HmiData &data);

    /**
     * @brief This function is used to know if a registered
     * verb needs a noun for its execution or it can be
     * executed by itself
     * 
     * @param verb[in] Verb
     *
     * @return true If Noun is expected for this Verb
     * @return false If any Noun is expected
     */
    bool needNoun(int verb);

    /**
     * @brief Human-readable description of every registered action,
     *        sorted by (verb, noun) ascending. Used to build the VERB 0 menu.
     *
     * @return One string per action, e.g. "V01 CLOCK" or "V02 N00 KEYPAD OFF"
     */
    std::vector<std::string> describe() const;

};

#endif /* _LISTOFACTIONS_H_ */