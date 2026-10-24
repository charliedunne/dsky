#ifndef _ACTIONVERBLIST_H_
#define _ACTIONVERBLIST_H_

#include "Action.h"

// Action registry
#include "ListOfActions.h"

class ActionVerbList : public Action {

 private:

  /**
   * @brief Reference to the registered actions (source of the VERB 0 menu)
   */
  const ListOfActions &actions_;

 public:

  ActionVerbList(HmiData &data, const ListOfActions &actions);

  void operation();
};

#endif /* _ACTIONVERBLIST_H_ */
