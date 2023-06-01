#ifndef _LISTOFACTIONS_H_
#define _LISTOFACTIONS_H_

// Generic Action
#include "Action.h"

// STL
#include <vector>

class ListOfActions {

    private:

    /**
     * @brief Array of actions
     */
    std::vector<Action *> actions_;

    public:

    ListOfActions();
    virtual ~ListOfActions();

    /**
     * @brief Register a new action in the list
     * 
     * @param action[in] Pointer to the action to register 
     */
    void registerAction(Action * action);

    /**
     * @brief Overload to the operator() to make a functor
     * 
     * @param verb[in] Verb 
     * @param noun[in] Noun 
     */
    void operator()(int verb, int noun);
};

#endif /* _LISTOFACTIONS_H_ */