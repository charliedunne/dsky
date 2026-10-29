#ifndef _DISABLESCREENSAVER_H_
#define _DISABLESCREENSAVER_H_

#include "Action.h"
#include "ScreenSaver.h"

class DisableScreenSaver : public Action {

 public:

  DisableScreenSaver(HmiData & data, ScreenSaver & saver);

  void operation();

 private:

  /**
   * @brief Reference to the shared screensaver instance
   */
  ScreenSaver & saver_;
};

#endif /* _DISABLESCREENSAVER_H_ */
