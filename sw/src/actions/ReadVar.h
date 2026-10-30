#ifndef _READVAR_H_
#define _READVAR_H_

#include "Action.h"
#include "VariableStore.h"

class ReadVar : public Action {

 public:

  /**
   * @brief Constructor
   *
   * @param[in] store Persistent variable store to read from
   * @param[in] noun NOUN selector identifying the target variable
   * @param[in] data HMI data (target of the displayed value)
   */
  ReadVar(VariableStore &store, int noun, HmiData &data);

  void operation();

 private:
  VariableStore &store_; ///< Reference to the persistent variable store
};

#endif /* _READVAR_H_ */
