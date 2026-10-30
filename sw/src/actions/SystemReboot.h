#ifndef _SYSTEMREBOOT_H_
#define _SYSTEMREBOOT_H_

#include "Action.h"

// STL
#include <atomic>

class SystemReboot : public Action {

 public:

  SystemReboot(HmiData & data);

  void operation();

  /**
   * @brief The reboot countdown can be aborted by any key press
   */
  bool cancellable() const override;

  /**
   * @brief Abort the countdown. No reboot is issued and the
   *        left panel is cleared by the action thread itself.
   */
  void cancel() override;

 private:

  /**
   * @brief Countdown length before the actual reboot [seconds]
   */
  static constexpr int COUNTDOWN_SECONDS = 10;

  /**
   * @brief Request to abort the countdown (set by cancel())
   */
  std::atomic<bool> cancel_;
};

#endif /* _SYSTEMREBOOT_H_ */
