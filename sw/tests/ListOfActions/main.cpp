#include "../common/test.h"
#include "Action.h"
#include "ListOfActions.h"
#include "ActionVerbList.h"
#include "ExcInvalidCommand.h"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

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

TEST(need_noun) {
  HmiData data;
  TestAction a(1, -1, data);
  TestAction b(2, 0, data);
  TestAction c(3, 5, data);
  ListOfActions list;
  list.registerAction(a);
  list.registerAction(b);
  list.registerAction(c);

  CHECK_EQ(list.needNoun(1), false);
  CHECK_EQ(list.needNoun(2), true);
  CHECK_EQ(list.needNoun(3), true);

  // Unregistered verb: throws ExcInvalidCommand specifically.
  bool threw = false;
  try { list.needNoun(99); } catch (const ExcInvalidCommand &) { threw = true; }
  CHECK(threw);
}

TEST(lookup_by_verb_noun) {
  HmiData data;
  TestAction a(1, -1, data);
  TestAction b(2, 0, data);
  ListOfActions list;
  list.registerAction(a);
  list.registerAction(b);

  // Registered (verb, noun): returns the exact registered object.
  data.rSetnVerb(2, DRAW_ON);
  data.rSetnNoun(0, DRAW_ON);
  Action &found = list(data);
  CHECK(&found == &b);

  // Unknown verb: throws ExcInvalidCommand specifically.
  data.rSetnVerb(42, DRAW_ON);
  data.rSetnNoun(0, DRAW_ON);
  bool threw = false;
  try { list(data); } catch (const ExcInvalidCommand &) { threw = true; }
  CHECK(threw);

  // Fresh HmiData: nVerb/nNoun are UNASIGNED -> not registered.
  HmiData fresh;
  bool threwFresh = false;
  try { list(fresh); } catch (const ExcInvalidCommand &) { threwFresh = true; }
  CHECK(threwFresh);
}

TEST(describe_sorted_and_formatted) {
  HmiData data;
  TestAction a(1, -1, data, "CLOCK");
  TestAction b(2, 0, data, "KEYPAD OFF");
  TestAction c(0, -1, data, "VERB LIST");
  TestAction d(2, 7, data, "OTHER");

  ListOfActions list;
  // Register in scrambled order; describe() must sort by (verb, noun).
  list.registerAction(b);
  list.registerAction(a);
  list.registerAction(d);
  list.registerAction(c);

  std::vector<std::string> desc = list.describe();
  CHECK_EQ(desc.size(), 4u);
  if (desc.size() == 4) {
    CHECK_EQ(desc[0], "V00 VERB LIST");
    CHECK_EQ(desc[1], "V01 CLOCK");
    CHECK_EQ(desc[2], "V02 N00 KEYPAD OFF");
    CHECK_EQ(desc[3], "V02 N07 OTHER");
  }
}

TEST(verb_list_action) {
  HmiData data;
  TestAction a(1, -1, data, "CLOCK");
  TestAction b(2, 0, data, "KEYPAD OFF");
  ListOfActions list;
  list.registerAction(a);
  list.registerAction(b);

  ActionVerbList vl(data, list);
  CHECK_EQ(vl.getVerb(), 0);
  CHECK_EQ(vl.getNoun(), -1);
  CHECK_EQ(vl.getLabel(), "VERB LIST");
  CHECK_EQ(vl.getSchedule(), ACTION_SINGLE);

  vl.run();
  CHECK(waitFinished(vl));
  CHECK(vl.finished());

  const LeftLcdData &l = data.getLeftLcdData();
  CHECK_EQ(l.lVerbList, DRAW_ON);
  CHECK_EQ(l.lVerbListCount, 2);
  CHECK_EQ(std::string(l.lVerbListLines[0]), "V01 CLOCK");
  CHECK_EQ(std::string(l.lVerbListLines[1]), "V02 N00 KEYPAD OFF");

  // Reap the joinable worker thread before it goes out of scope.
  vl.stop();
}

TESTS_MAIN;
