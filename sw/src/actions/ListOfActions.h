#ifndef _LISTOFACTIONS_H_
#define _LISTOFACTIONS_H_

// Generic Action
#include "Action.h"

// STL
#include <vector>
#include <string>

// List of VERBS/NOUNS

#define ACTION_LIST_VERB 0
#define ACTION_LIST_NOUN -1

#define CLOCK_VERB 1
#define CLOCK_NOUN -1

/* VERBS 2 and 3 were removed: keypad light and screensaver are now
 * controlled through the variable read/write verbs (READ_VAR/WRITE_VAR). */

#define READ_VAR_VERB 8
#define WRITE_VAR_VERB 9

/* Nouns used by the variable read/write verbs */
#define VAR_NOUN_KEYPAD_LIGHT 2
#define VAR_NOUN_SCREENSAVER 3
#define VAR_NOUN_SCREENSAVER_TIMER 4

#define TEMP_MONITOR_VERB 10
#define TEMP_MONITOR_NOUN -1

/*
 * Add here more Actions
 */

#define REBOOT_VERB 98
#define REBOOT_NOUN -1

#define SHUTDOWN_VERB 99
#define SHUTDOWN_NOUN -1



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
     * @brief Check if a registered (verb, noun) command requires a value input.
     *
     * Only the write-variable verb prompts for a value, and only when an
     * action is actually registered for that exact (verb, noun) pair.
     *
     * @param verb[in] Verb
     * @param noun[in] Noun
     *
     * @return true If a value must be typed before execution
     * @return false If no value input is required
     */
    bool needValue(int verb, int noun);

    /**
     * @brief Human-readable description of every registered action,
     *        sorted by (verb, noun) ascending. Used to build the VERB 0 menu.
     *
     * @return One string per action, e.g. "V01 CLOCK" or "V02 N00 KEYPAD OFF"
     */
    std::vector<std::string> describe() const;

};

#endif /* _LISTOFACTIONS_H_ */