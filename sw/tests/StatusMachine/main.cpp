#include "../common/test.h"
#include "StatusMachine.h"
#include <string>

// Drive a fresh machine into the given mode using only documented valid
// transitions.
static void driveTo(StatusMachine &m, Mode_e target) {
  switch (target) {
  case MODE_IDLE:
    break;
  case MODE_V_INPUT:
    m.transit(MODE_V_INPUT);
    break;
  case MODE_N_INPUT:
    m.transit(MODE_V_INPUT);
    m.transit(MODE_N_INPUT);
    break;
  case MODE_RUN:
    m.transit(MODE_V_INPUT);
    m.transit(MODE_N_INPUT);
    m.transit(MODE_RUN);
    break;
  case MODE_ERROR:
    m.transit(MODE_ERROR);
    break;
  }
}

TEST(initial_mode_is_idle) {
  StatusMachine m;
  CHECK_EQ(m(), MODE_IDLE);
  CHECK_EQ(m.to_string(), std::string("IDLE"));
}

TEST(valid_transitions) {
  // IDLE -> V_INPUT -> N_INPUT -> RUN -> IDLE
  {
    StatusMachine m;
    m.transit(MODE_V_INPUT);
    CHECK_EQ(m(), MODE_V_INPUT);
    m.transit(MODE_N_INPUT);
    CHECK_EQ(m(), MODE_N_INPUT);
    m.transit(MODE_RUN);
    CHECK_EQ(m(), MODE_RUN);
    m.transit(MODE_IDLE);
    CHECK_EQ(m(), MODE_IDLE);
  }
  // N_INPUT -> V_INPUT, then V_INPUT -> IDLE
  {
    StatusMachine m;
    m.transit(MODE_V_INPUT);
    m.transit(MODE_N_INPUT);
    m.transit(MODE_V_INPUT);
    CHECK_EQ(m(), MODE_V_INPUT);
    m.transit(MODE_IDLE);
    CHECK_EQ(m(), MODE_IDLE);
  }
  // IDLE -> ERROR, then ERROR -> IDLE
  {
    StatusMachine m;
    m.transit(MODE_ERROR);
    CHECK_EQ(m(), MODE_ERROR);
    m.transit(MODE_IDLE);
    CHECK_EQ(m(), MODE_IDLE);
  }
}

TEST(invalid_transitions_throw) {
  // From RUN: V_INPUT and N_INPUT are invalid.
  {
    StatusMachine m;
    driveTo(m, MODE_RUN);
    CHECK_THROWS(m.transit(MODE_V_INPUT));
    CHECK_THROWS(m.transit(MODE_N_INPUT));
  }
  // From IDLE: RUN is invalid, as is an unknown destination mode.
  {
    StatusMachine m;
    CHECK_THROWS(m.transit(MODE_RUN));
    CHECK_THROWS(m.transit((Mode_e)999));
  }
}

TEST(self_transitions_are_noops) {
  const Mode_e all[] = {MODE_IDLE, MODE_V_INPUT, MODE_N_INPUT, MODE_RUN,
                        MODE_ERROR};
  for (Mode_e mode : all) {
    StatusMachine m;
    driveTo(m, mode);
    m.transit(mode); // must not throw
    CHECK_EQ(m(), mode);
  }
}

TEST(to_string_for_each_mode) {
  struct Entry {
    Mode_e mode;
    const char *name;
  };
  const Entry table[] = {{MODE_IDLE, "IDLE"},
                         {MODE_V_INPUT, "V_INPUT"},
                         {MODE_N_INPUT, "N_INPUT"},
                         {MODE_RUN, "RUN"},
                         {MODE_ERROR, "ERROR"}};
  for (const auto &e : table) {
    StatusMachine m;
    driveTo(m, e.mode);
    CHECK_EQ(m.to_string(), std::string(e.name));
  }
}

TESTS_MAIN;
