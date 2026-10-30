#ifndef _ACTIONVERBLIST_H_
#define _ACTIONVERBLIST_H_

#include "Action.h"

// Action registry
#include "ListOfActions.h"

// Persistent variable store (source of the VERB 0 variables page)
#include "VariableStore.h"

class ActionVerbList : public Action {

 private:

  /**
   * @brief Reference to the registered actions (source of the VERB 0 menu)
   */
  const ListOfActions &actions_;

  /**
   * @brief Reference to the variable store (source of the variables page)
   */
  const VariableStore &store_;

 public:

  ActionVerbList(HmiData &data, const ListOfActions &actions, const VariableStore &store);

  void operation();
};

#endif /* _ACTIONVERBLIST_H_ */
