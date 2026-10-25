#include "../common/test.h"
#include "Action.h"
#include "ActionClock.h"

#include <chrono>
#include <thread>

// Minimal concrete Action for exercising the base class.
class TestAction : public Action {
 public:
  TestAction(int verb, int noun, HmiData &d, const std::string &label = "TEST")
      : Action(verb, noun, d, ACTION_SINGLE, label) {}
  void operation() override {}
};

// Poll finished() for up to ~2 seconds (single-shot completion is async).
static bool waitFinished(Action &a) {
  for (int i = 0; i < 200; ++i) {
    if (a.finished()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return a.finished();
}

TEST(ctor_defaults_and_getters) {
  HmiData data;
  TestAction t(7, 3, data);
  CHECK_EQ(t.getVerb(), 7);
  CHECK_EQ(t.getNoun(), 3);
  CHECK_EQ(t.getLabel(), "TEST");
  CHECK_EQ(t.getSchedule(), ACTION_SINGLE);

  // Default label is the empty string.
  TestAction d(1, -1, data, "");
  CHECK_EQ(d.getLabel(), "");
}

TEST(set_period_forces_periodic) {
  HmiData data;
  TestAction t(7, 3, data);
  CHECK_EQ(t.getSchedule(), ACTION_SINGLE);
  t.setPeriod(50);
  CHECK_EQ(t.getSchedule(), ACTION_PERIODIC);
}

TEST(single_action_lifecycle) {
  HmiData data;
  TestAction t(7, 3, data);
  t.run();
  CHECK(waitFinished(t));
  CHECK(t.finished());
  CHECK(!t.isRunning());
  // Reap the joinable worker thread before it goes out of scope.
  t.stop();
}

TEST(single_action_rerun_after_stop) {
  // Regression test: run() must reap a previous JOINABLE thread, otherwise
  // assigning to a joinable std::thread calls std::terminate.
  HmiData data;
  TestAction t(7, 3, data);

  // First cycle.
  t.run();
  CHECK(waitFinished(t));
  CHECK(t.finished());
  t.stop();

  // Second cycle on the same object (the app does this: stop a finished
  // single-shot and re-run it next frame).
  t.run();
  CHECK(waitFinished(t));
  CHECK(t.finished());
  CHECK(!t.isRunning());
  t.stop();
}

TEST(clock_metadata) {
  HmiData data;
  ActionClock c(data);
  CHECK_EQ(c.getVerb(), 1);
  CHECK_EQ(c.getNoun(), -1);
  CHECK_EQ(c.getLabel(), "CLOCK");
  CHECK_EQ(c.getSchedule(), ACTION_PERIODIC);
}

TEST(clock_periodic_updates_lcd) {
  HmiData data;
  const RightLcdData &r0 = data.getRightLcdData();
  CHECK_EQ(r0.nR1, UNASIGNED);

  ActionClock c(data);
  c.run();
  // Period is 1000 ms; give it a bit of margin so at least one tick lands.
  std::this_thread::sleep_for(std::chrono::milliseconds(1300));
  CHECK(c.isRunning());

  const RightLcdData &r = data.getRightLcdData();
  CHECK(r.nR1 != UNASIGNED && r.nR1 >= 0 && r.nR1 <= 23);
  CHECK(r.nR2 != UNASIGNED && r.nR2 >= 0 && r.nR2 <= 59);
  CHECK(r.nR3 != UNASIGNED && r.nR3 >= 0 && r.nR3 <= 59);
  // Value modes.
  CHECK_EQ(r.nR1Mode, DRAW_ON);
  CHECK_EQ(r.nR2Mode, DRAW_ON);
  CHECK_EQ(r.nR3Mode, DRAW_ON);
  // Label modes.
  CHECK_EQ(r.lR1Mode, DRAW_ON);
  CHECK_EQ(r.lR2Mode, DRAW_ON);
  CHECK_EQ(r.lR3Mode, DRAW_ON);
  CHECK_EQ(r.lCompActyMode, DRAW_ON);

  c.stop();
  CHECK(!c.isRunning());
}

TESTS_MAIN;
