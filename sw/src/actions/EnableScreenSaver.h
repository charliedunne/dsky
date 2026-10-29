#ifndef _ENABLESCREENSAVER_H_
#define _ENABLESCREENSAVER_H_

#include "Action.h"
#include "ScreenSaver.h"

class EnableScreenSaver : public Action {

 public:

  EnableScreenSaver(HmiData & data, ScreenSaver & saver);

  void operation();

 private:

  /**
   * @brief Reference to the shared screensaver instance
   */
  ScreenSaver & saver_;
};

#endif /* _ENABLESCREENSAVER_H_ */
