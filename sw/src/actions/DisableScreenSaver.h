#ifndef _DISABLESCREENSAVER_H_
#define _DISABLESCREENSAVER_H_

#include "Action.h"
#include "VariableStore.h"

class DisableScreenSaver : public Action {

 public:

  /**
   * @brief Constructor
   *
   * @param[in] store Persistent variable store to write into
   * @param[in] data HMI data
   */
  DisableScreenSaver(VariableStore &store, HmiData &data);

  void operation();

 private:
  VariableStore &store_; ///< Reference to the persistent variable store
};

#endif /* _DISABLESCREENSAVER_H_ */
