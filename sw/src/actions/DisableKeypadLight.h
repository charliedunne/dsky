#ifndef _DISABLEKEYPADLIGHT_H_
#define _DISABLEKEYPADLIGHT_H_

#include "Action.h"

class DisableKeypadLight : public Action {

 public:

  DisableKeypadLight(HmiData & data);

  void operation();
};

#endif /* _DISABLEKEYPADLIGHT_H_ */