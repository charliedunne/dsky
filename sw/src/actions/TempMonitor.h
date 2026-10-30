#ifndef _TEMPMONITOR_H_
#define _TEMPMONITOR_H_

#include "Action.h"

class TempMonitor : public Action {

 public:

  TempMonitor(HmiData & data);

  void operation();
};

#endif /* _TEMPMONITOR_H_ */
