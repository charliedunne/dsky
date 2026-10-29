#ifndef _ENABLEKEYPADLIGHT_H_
#define _ENABLEKEYPADLIGHT_H_

#include "Action.h"
#include "VariableStore.h"

class EnableKeypadLight : public Action {

 public:

  /**
   * @brief Constructor
   *
   * @param[in] store Persistent variable store to write into
   * @param[in] data HMI data
   */
  EnableKeypadLight(VariableStore &store, HmiData &data);

  void operation();

 private:
  VariableStore &store_; ///< Reference to the persistent variable store
};

#endif /* _ENABLEKEYPADLIGHT_H_ */
