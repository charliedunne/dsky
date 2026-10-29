#ifndef _DISABLEKEYPADLIGHT_H_
#define _DISABLEKEYPADLIGHT_H_

#include "Action.h"
#include "VariableStore.h"

class DisableKeypadLight : public Action {

 public:

  /**
   * @brief Constructor
   *
   * @param[in] store Persistent variable store to write into
   * @param[in] data HMI data
   */
  DisableKeypadLight(VariableStore &store, HmiData &data);

  void operation();

 private:
  VariableStore &store_; ///< Reference to the persistent variable store
};

#endif /* _DISABLEKEYPADLIGHT_H_ */
