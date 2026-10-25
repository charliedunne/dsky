#include "../common/test.h"
#include "Event.h"
#include <cstdint>
#include <ctime>

TEST(ctor_with_event) {
  std::time_t t_before = std::time(nullptr);
  Event e(E_KEY_VERB);
  std::time_t t_after = std::time(nullptr);
  CHECK_EQ(e.getEvent(), E_KEY_VERB);
  CHECK(t_before <= e.getTimestamp() && e.getTimestamp() <= t_after);
}

TEST(record_changes_event_and_time) {
  Event e(E_KEY_0);
  std::time_t t1 = e.getTimestamp();
  e.record(E_KEY_ENTR);
  CHECK_EQ(e.getEvent(), E_KEY_ENTR);
  CHECK(e.getTimestamp() >= t1);
}

TEST(key_code_values) {
  // Exact protocol key codes (cast to uint32_t to avoid enum underlying-type surprises).
  CHECK_EQ(static_cast<uint32_t>(E_KEY_0), 0xf7ffffffu);
  CHECK_EQ(static_cast<uint32_t>(E_KEY_9), 0xffffefffu);
  CHECK_EQ(static_cast<uint32_t>(E_KEY_VERB), 0x7fffffffu);
  CHECK_EQ(static_cast<uint32_t>(E_KEY_NOUN), 0xbfffffffu);
  CHECK_EQ(static_cast<uint32_t>(E_KEY_CLR), 0xfffffffbu);
  CHECK_EQ(static_cast<uint32_t>(E_KEY_ENTR), 0xffffffdfu);
  CHECK_EQ(static_cast<uint32_t>(E_NO_EVENT), 0xffffffffu);
}

TESTS_MAIN;
