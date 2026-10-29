#ifndef _ENABLESCREENSAVER_H_
#define _ENABLESCREENSAVER_H_

#include "Action.h"
#include "VariableStore.h"

class EnableScreenSaver : public Action {

 public:

  /**
   * @brief Constructor
   *
   * @param[in] store Persistent variable store to write into
   * @param[in] data HMI data
   */
  EnableScreenSaver(VariableStore &store, HmiData &data);

  void operation();

 private:
  VariableStore &store_; ///< Reference to the persistent variable store
};

#endif /* _ENABLESCREENSAVER_H_ */
