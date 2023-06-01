#include "ActionClock.h"

#include <iostream>

int main() {

  HmiData data;

  ActionClock clockAction(&data);

  clockAction.setPeriod(1000);

  std::cout << "Data: " << data.nR1 << std::endl;
  clockAction.run();
  std::cout << "Data: " << data.nR1 << std::endl;



  return 0;

}
