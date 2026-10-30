#ifndef _WRITEVAR_H_
#define _WRITEVAR_H_

#include "Action.h"
#include "VariableStore.h"

class WriteVar : public Action {

 public:

  /**
   * @brief Constructor
   *
   * @param[in] store Persistent variable store to write into
   * @param[in] noun NOUN selector identifying the target variable
   * @param[in] data HMI data (source of the typed value)
   */
  WriteVar(VariableStore &store, int noun, HmiData &data);

  void operation();

 private:
  VariableStore &store_; ///< Reference to the persistent variable store
};

#endif /* _WRITEVAR_H_ */
