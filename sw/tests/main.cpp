#include "ActionClock.h"

int main() {

  HmiData data;

  ActionClock clockAction(&data);

  clockAction.setPeriod(2);

  clockAction.run();

  return 0;

}
