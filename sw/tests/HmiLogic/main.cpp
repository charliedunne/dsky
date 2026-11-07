// HmiLogic state-machine regression suite (step 3).
//
// HmiLogic is a black box: the StatusMachine has no getter, so every case
// asserts state only through HmiData side effects and subsequent behavior.
//
// Hygiene law: never let a case end with an action still running (~Action on
// a joinable thread calls std::terminate and HmiLogic's destructor does not
// stop actions). Every case below ends either reaped (settle) or stopped
// (RSET path).

#include "../common/test.h"
#include "HmiLogic.h"
#include "ActionVerbList.h" // pulls Action.h, ListOfActions.h, HmiData.h, Event.h
#include "VerbListView.h"

#include <chrono>
#include <thread>
#include <vector>

// One key event followed by one logic frame.
static void press(HmiLogic &logic, HmiData &data, EventId_t key) {
  std::vector<Event> ev;
  ev.push_back(Event(key));
  logic.parseKeyEvents(ev);
  logic.updateHmiData(data);
}

// A frame with no key (the app runs one per tick).
static void frame(HmiLogic &logic, HmiData &data) {
  logic.parseKeyEvents({});
  logic.updateHmiData(data);
}

// Wait for a single-shot action to finish, then run one no-key frame so
// HmiLogic reaps it (stop + resetRightLcdData + IDLE). Final state is
// deterministic regardless of the double-run race in updateHmiData.
static void settle(HmiLogic &logic, HmiData &data) {
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  frame(logic, data);
}

// Minimal no-op single-shot action.
class TestAction : public Action {
 public:
  TestAction(int verb, int noun, HmiData &d, const std::string &label = "TEST")
      : Action(verb, noun, d, ACTION_SINGLE, label) {}
  void operation() override {}
};

// Periodic action that marks nR1 every tick (used to verify RSET stops it).
class MarkerAction : public Action {
 public:
  MarkerAction(HmiData &d) : Action(2, -1, d, ACTION_SINGLE, "MARKER") {
    setPeriod(50);
  }
  void operation() override { hmiData_.rSetnR1(42, DRAW_ON); }
};

TEST(idle_verb_key_enters_verb_input) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  press(logic, data, E_KEY_VERB);
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.lVerbMode, DRAW_BLINK); // input-mode labels blink while typing
  CHECK_EQ(r.nVerb, UNASIGNED); // no digit yet
  CHECK_EQ(r.lNounMode, DRAW_OFF);
}

TEST(idle_nonverb_key_clears_verb_list) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  data.showVerbList({"V00 VERB LIST", "V01 TEST ONE"});
  data.rSetnR1(7, DRAW_ON);
  press(logic, data, E_KEY_1);
  CHECK(data.getLeftView() == nullptr); // menu dismissed
  // The idle clear also dismisses right-side residue (list-verb label/number).
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.nR1, UNASIGNED);
  CHECK_EQ(r.nR1Mode, DRAW_OFF);

  // Second part: menu already OFF - a stray key is a plain no-op.
  HmiData d2;
  VariableStore store2("unused");
  HmiLogic l2(store2);
  press(l2, d2, E_KEY_5);
  CHECK_EQ(d2.getRightLcdData().nVerb, UNASIGNED);
}

TEST(idle_verb_key_keeps_verb_list) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  data.showVerbList({"V00 VERB LIST"});
  press(logic, data, E_KEY_VERB);
  const VerbListView *v = data.leftViewAs<VerbListView>();
  CHECK(v != nullptr); // menu preserved
  if (v) CHECK_EQ(v->pages[0].size(), 1u);
  // VERB took priority over the clear branch: we are in V_INPUT now.
  CHECK_EQ(data.getRightLcdData().lVerbMode, DRAW_BLINK);
}

TEST(verb_input_digit_accumulation_and_invalid_verb_errors) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a1(1, -1, data, "ONE");
  logic.registerAction(a1);

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_2);
  CHECK_EQ(data.getRightLcdData().nVerb, 2);
  CHECK_EQ(data.getRightLcdData().nVerbMode, DRAW_ON);
  press(logic, data, E_KEY_5);
  CHECK_EQ(data.getRightLcdData().nVerb, 25); // accumulation: 2*10+5

  // Verb 25 is unregistered: needNoun throws -> ERROR.
  press(logic, data, E_KEY_ENTR);
  CHECK_EQ(data.getLeftAlarms().lOpErr, DRAW_BLINK);
}

TEST(verb_input_clr_and_verb_key_reclear) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a1(1, -1, data, "ONE");
  logic.registerAction(a1);

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_5);
  CHECK_EQ(data.getRightLcdData().nVerb, 5);
  CHECK_EQ(data.getRightLcdData().nVerbMode, DRAW_ON);

  // CLR clears the verb but stays in V_INPUT.
  press(logic, data, E_KEY_CLR);
  CHECK_EQ(data.getRightLcdData().nVerb, UNASIGNED);
  CHECK_EQ(data.getRightLcdData().nVerbMode, DRAW_OFF);

  // Fresh start: 3, NOT 53 (proves the clear worked and we stayed in V_INPUT).
  press(logic, data, E_KEY_3);
  CHECK_EQ(data.getRightLcdData().nVerb, 3);

  // VERB key re-clears as well.
  press(logic, data, E_KEY_VERB);
  CHECK_EQ(data.getRightLcdData().nVerb, UNASIGNED);
  CHECK_EQ(data.getRightLcdData().nVerbMode, DRAW_OFF);
}

TEST(verb_input_stray_key_cancels_to_idle_left_untouched) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a1(1, -1, data, "ONE");
  logic.registerAction(a1);

  data.lSetUplinkAct(DRAW_ON);
  data.showVerbList({"V01 TEST ONE"});

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_5);
  CHECK_EQ(data.getRightLcdData().nVerb, 5);

  // Stray key: resetRightLcdData ONLY + IDLE (left LCD untouched).
  press(logic, data, E_KEY_PLUS);
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.nVerb, UNASIGNED);
  CHECK_EQ(r.nVerbMode, DRAW_OFF);
  const LeftAlarmFlags l = data.getLeftAlarms();
  CHECK_EQ(l.lUplinkAct, DRAW_ON);
  CHECK(data.leftViewAs<VerbListView>() != nullptr); // menu untouched
  CHECK_EQ(l.lOpErr, DRAW_OFF);

  // Observe we are in IDLE: a digit key clears the menu via idle().
  press(logic, data, E_KEY_1);
  CHECK(data.getLeftView() == nullptr);
}

TEST(noun_input_flow_and_cancel) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a37(3, 7, data, "N7");
  logic.registerAction(a37);

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_3);
  CHECK_EQ(data.getRightLcdData().nVerb, 3);
  CHECK_EQ(data.getRightLcdData().nVerbMode, DRAW_ON);
  // Verb 3 needs a noun -> N_INPUT.
  press(logic, data, E_KEY_ENTR);
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.lNounMode, DRAW_BLINK); // input-mode labels blink while typing
  CHECK_EQ(r.nVerb, 3);
  CHECK_EQ(r.nVerbMode, DRAW_ON);

  press(logic, data, E_KEY_7);
  CHECK_EQ(data.getRightLcdData().nNoun, 7);
  CHECK_EQ(data.getRightLcdData().nNounMode, DRAW_ON);

  // NOUN clears the noun but stays in N_INPUT.
  press(logic, data, E_KEY_NOUN);
  CHECK_EQ(data.getRightLcdData().nNoun, UNASIGNED);
  CHECK_EQ(data.getRightLcdData().nNounMode, DRAW_OFF);

  // Fresh start: 8, NOT 78.
  press(logic, data, E_KEY_8);
  CHECK_EQ(data.getRightLcdData().nNoun, 8);
}

TEST(noun_input_verb_key_quirk_and_recovery) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a37(3, 7, data, "N7");
  TestAction a1(1, -1, data, "ONE");
  logic.registerAction(a37);
  logic.registerAction(a1);

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_3);
  press(logic, data, E_KEY_ENTR); // -> N_INPUT
  CHECK_EQ(data.getRightLcdData().lNounMode, DRAW_BLINK); // input-mode labels blink while typing

  // KNOWN QUIRK: the E_KEY_VERB case in nounInput() is missing its break and
  // falls through to the NOUN/CLR case, which redundantly re-clears nNoun.
  // Net effect (pinned here): verb + noun + noun label cleared, back in V_INPUT.
  press(logic, data, E_KEY_VERB);
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.nVerb, UNASIGNED);
  CHECK_EQ(r.nVerbMode, DRAW_OFF);
  CHECK_EQ(r.nNoun, UNASIGNED);
  CHECK_EQ(r.nNounMode, DRAW_OFF);
  CHECK_EQ(r.lNounMode, DRAW_OFF);

  // Recover: verb 1 needs no noun -> RUN; settle reaps the action.
  press(logic, data, E_KEY_1);
  CHECK_EQ(data.getRightLcdData().nVerb, 1);
  CHECK_EQ(data.getRightLcdData().nVerbMode, DRAW_ON);
  press(logic, data, E_KEY_ENTR);
  settle(logic, data);
  CHECK_EQ(data.getLeftAlarms().lOpErr, DRAW_OFF);
  CHECK_EQ(data.getRightLcdData().nVerb, UNASIGNED); // reaped -> right reset
}

TEST(noun_input_stray_key_cancels_to_idle_resets_both) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a37(3, 7, data, "N7");
  logic.registerAction(a37);

  data.lSetUplinkAct(DRAW_ON);
  data.showVerbList({"V03 N07 N7"});

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_3);
  press(logic, data, E_KEY_ENTR); // -> N_INPUT

  // Stray key: full reset of BOTH LCDs + IDLE (the VERB 7 program list must
  // not survive a cancel).
  press(logic, data, E_KEY_PRO);
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.nVerb, UNASIGNED);
  CHECK_EQ(r.nVerbMode, DRAW_OFF);
  CHECK_EQ(r.lNounMode, DRAW_OFF);
  const LeftAlarmFlags l = data.getLeftAlarms();
  CHECK_EQ(l.lUplinkAct, DRAW_OFF);
  CHECK(data.getLeftView() == nullptr); // menu reset along with the flags

  // Observe we are in IDLE: VERB starts a fresh input (label blinks).
  press(logic, data, E_KEY_VERB);
  CHECK_EQ(data.getRightLcdData().lVerbMode, DRAW_BLINK);
}

TEST(noun_input_enter_without_noun_errors) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a37(3, 7, data, "N7");
  logic.registerAction(a37);

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_3);
  press(logic, data, E_KEY_ENTR); // -> N_INPUT, nNoun still UNASIGNED
  CHECK_EQ(data.getRightLcdData().nNoun, UNASIGNED);

  // ENTER without a noun: lookup (3, -1) misses -> ExcInvalidCommand -> ERROR.
  press(logic, data, E_KEY_ENTR);
  CHECK_EQ(data.getLeftAlarms().lOpErr, DRAW_BLINK);

  // Any key clears the error: full reset back to IDLE.
  press(logic, data, E_KEY_9);
  CHECK_EQ(data.getLeftAlarms().lOpErr, DRAW_OFF);
  CHECK_EQ(data.getRightLcdData().nVerb, UNASIGNED);
  CHECK_EQ(data.getRightLcdData().nVerbMode, DRAW_OFF);

  // Observe we are in IDLE: VERB starts a fresh input.
  press(logic, data, E_KEY_VERB);
  CHECK_EQ(data.getRightLcdData().lVerbMode, DRAW_BLINK);
}

TEST(error_any_key_returns_to_clean_idle) {
  // THE core regression (PR #2 fix #1): from ERROR any key must return to a
  // fully clean IDLE state.
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a1(1, -1, data, "ONE");
  logic.registerAction(a1);

  data.lSetUplinkAct(DRAW_ON);
  data.showVerbList({"V01 TEST ONE"});

  // Trigger ERROR with an unregistered verb.
  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_9);
  CHECK_EQ(data.getRightLcdData().nVerb, 9);
  press(logic, data, E_KEY_ENTR); // needNoun(9) throws -> ERROR
  const LeftAlarmFlags l0 = data.getLeftAlarms();
  CHECK_EQ(l0.lOpErr, DRAW_BLINK);
  // The error path itself must not touch the rest of the left LCD.
  CHECK_EQ(l0.lUplinkAct, DRAW_ON);
  CHECK(data.leftViewAs<VerbListView>() != nullptr);

  // A "stray" key (any key) clears the error: full clean state.
  press(logic, data, E_KEY_PLUS);
  const LeftAlarmFlags l = data.getLeftAlarms();
  CHECK_EQ(l.lOpErr, DRAW_OFF);
  CHECK_EQ(l.lUplinkAct, DRAW_OFF);
  CHECK(data.getLeftView() == nullptr);
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.nVerb, UNASIGNED);
  CHECK_EQ(r.nVerbMode, DRAW_OFF);

  // Observe we are in IDLE: VERB starts a fresh input again.
  press(logic, data, E_KEY_VERB);
  CHECK_EQ(data.getRightLcdData().lVerbMode, DRAW_BLINK);
}

TEST(verb0_end_to_end) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a1(1, -1, data, "TEST ONE");
  TestAction a37(3, 7, data, "TEST N7");
  logic.registerAction(a1);
  logic.registerAction(a37);
  // Mirror the app wiring: register VERB LIST last (empty var/program pages).
  ListOfPrograms programs;
  ActionVerbList a_VerbList(data, logic.getActions(), store, programs);
  logic.registerAction(a_VerbList);

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_0);
  CHECK_EQ(data.getRightLcdData().nVerb, 0);
  CHECK_EQ(data.getRightLcdData().nVerbMode, DRAW_ON);

  // Verb 0 needs no noun -> RUN; ActionVerbList builds the menu.
  press(logic, data, E_KEY_ENTR);
  settle(logic, data);

  const VerbListView *v = data.leftViewAs<VerbListView>();
  CHECK(v != nullptr);
  if (v) {
    CHECK_EQ(v->pages[0].size(), 3u);
    // describe() sorts by (verb, noun).
    CHECK(v->pages[0][0] == "V00 VERB LIST");
    CHECK(v->pages[0][1] == "V01 TEST ONE");
    CHECK(v->pages[0][2] == "V03 N07 TEST N7");
  }

  // List verbs (VERB 0) keep the right-side VERB label/number lit after reap.
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.nVerb, 0);
  CHECK_EQ(r.nVerbMode, DRAW_ON);

  // A stray key in IDLE with the menu ON clears it (idle()).
  press(logic, data, E_KEY_5);
  CHECK(data.getLeftView() == nullptr);
}

TEST(new_nonzero_command_dismisses_menu) {
  // Regression #3: a new non-VERB 0 command dismisses the VERB 0 menu.
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a1(1, -1, data, "TEST ONE");
  TestAction a37(3, 7, data, "TEST N7");
  logic.registerAction(a1);
  logic.registerAction(a37);
  ListOfPrograms programs;
  ActionVerbList a_VerbList(data, logic.getActions(), store, programs);
  logic.registerAction(a_VerbList);

  // First: full VERB 0 cycle so the menu is ON in IDLE.
  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_0);
  press(logic, data, E_KEY_ENTR);
  settle(logic, data);
  CHECK(data.leftViewAs<VerbListView>() != nullptr);

  // Now run verb 1 (no noun): clearLeftView() happens SYNCHRONOUSLY inside
  // the ENTER frame's run(), so assert immediately.
  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_1);
  press(logic, data, E_KEY_ENTR);
  CHECK(data.getLeftView() == nullptr);

  settle(logic, data);
  CHECK_EQ(data.getRightLcdData().nVerb, UNASIGNED); // reaped -> right reset
}

TEST(verb0_refresh_keeps_menu) {
  // Regression #4: re-running VERB 0 refreshes the menu without clearing it.
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  TestAction a1(1, -1, data, "TEST ONE");
  TestAction a37(3, 7, data, "TEST N7");
  logic.registerAction(a1);
  logic.registerAction(a37);
  ListOfPrograms programs;
  ActionVerbList a_VerbList(data, logic.getActions(), store, programs);
  logic.registerAction(a_VerbList);

  // First VERB 0 run.
  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_0);
  press(logic, data, E_KEY_ENTR);
  settle(logic, data);
  CHECK(data.leftViewAs<VerbListView>() != nullptr);

  // Second VERB 0 run: verb==0 -> NO clearLeftView; operation() rebuilds the
  // menu. (After the first reap the action is FRESH - stop() resets finished_
  // - so the second run really restarts it.)
  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_0);
  press(logic, data, E_KEY_ENTR);
  settle(logic, data);

  const VerbListView *v = data.leftViewAs<VerbListView>();
  CHECK(v != nullptr);
  if (v) {
    CHECK_EQ(v->pages[0].size(), 3u);
    CHECK(v->pages[0][0] == "V00 VERB LIST");
    CHECK(v->pages[0][1] == "V01 TEST ONE");
    CHECK(v->pages[0][2] == "V03 N07 TEST N7");
  }
}

TEST(rset_in_run_stops_periodic_action) {
  HmiData data;
  VariableStore store("unused");
  HmiLogic logic(store);
  MarkerAction marker(data); // verb 2, noun -1, periodic 50 ms
  logic.registerAction(marker);

  press(logic, data, E_KEY_VERB);
  press(logic, data, E_KEY_2);
  // Verb 2 needs no noun -> RUN; the periodic action ticks immediately.
  press(logic, data, E_KEY_ENTR);
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  CHECK_EQ(data.getRightLcdData().nR1, 42);
  CHECK_EQ(data.getRightLcdData().nR1Mode, DRAW_ON);

  data.lSetUplinkAct(DRAW_ON);

  // RSET in RUN: stop() + resetRight + resetLeft + IDLE.
  press(logic, data, E_KEY_RSET);
  const RightLcdData &r = data.getRightLcdData();
  CHECK_EQ(r.nR1, UNASIGNED);
  CHECK_EQ(r.nR1Mode, DRAW_OFF);
  CHECK_EQ(data.getLeftAlarms().lUplinkAct, DRAW_OFF);
  CHECK_EQ(r.lVerbMode, DRAW_OFF);

  // No more ticks: the periodic action really stopped.
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  CHECK_EQ(data.getRightLcdData().nR1, UNASIGNED);
}

TESTS_MAIN;
