#ifndef _ACTIONCLOCK_H_
#define _ACTIONCLOCK_H_

#include "Action.h"

class ActionClock : public Action {

 public:

  ActionClock(HmiData & data);

  void operation();
};

#endif /* _ACTIONCLOCK_H_ */
