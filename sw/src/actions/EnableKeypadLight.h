#ifndef _ENABLEKEYPADLIGHT_H_
#define _ENABLEKEYPADLIGHT_H_

#include "Action.h"

class EnableKeypadLight : public Action {

 public:

  EnableKeypadLight(HmiData & data);

  void operation();
};

#endif /* _ENABLEKEYPADLIGHT_H_ */