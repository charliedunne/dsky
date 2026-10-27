#ifndef _SYSTEMSHUTDOWN_H_
#define _SYSTEMSHUTDOWN_H_

#include "Action.h"

// STL
#include <atomic>

class SystemShutdown : public Action {

 public:

  SystemShutdown(HmiData & data);

  void operation();

  /**
   * @brief The shutdown countdown can be aborted by any key press
   */
  bool cancellable() const override;

  /**
   * @brief Abort the countdown. No poweroff is issued and the
   *        left panel is cleared by the action thread itself.
   */
  void cancel() override;

 private:

  /**
   * @brief Countdown length before the actual poweroff [seconds]
   */
  static constexpr int COUNTDOWN_SECONDS = 10;

  /**
   * @brief Request to abort the countdown (set by cancel())
   */
  std::atomic<bool> cancel_;
};

#endif /* _SYSTEMSHUTDOWN_H_ */
