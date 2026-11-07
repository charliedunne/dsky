#include "../common/test.h"
#include "HmiData.h"
#include "VerbListView.h"
#include <array>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

TEST(initial_state) {
  HmiData d;
  const RightLcdData &r = d.getRightLcdData();
  // Values
  CHECK_EQ(r.nProg, UNASIGNED);
  CHECK_EQ(r.nVerb, UNASIGNED);
  CHECK_EQ(r.nNoun, UNASIGNED);
  CHECK_EQ(r.nR1, UNASIGNED);
  CHECK_EQ(r.nR2, UNASIGNED);
  CHECK_EQ(r.nR3, UNASIGNED);
  // Value modes
  CHECK_EQ(r.nProgMode, DRAW_OFF);
  CHECK_EQ(r.nVerbMode, DRAW_OFF);
  CHECK_EQ(r.nNounMode, DRAW_OFF);
  CHECK_EQ(r.nR1Mode, DRAW_OFF);
  CHECK_EQ(r.nR2Mode, DRAW_OFF);
  CHECK_EQ(r.nR3Mode, DRAW_OFF);
  // Label modes
  CHECK_EQ(r.lCompActyMode, DRAW_OFF);
  CHECK_EQ(r.lProgMode, DRAW_OFF);
  CHECK_EQ(r.lVerbMode, DRAW_OFF);
  CHECK_EQ(r.lNounMode, DRAW_OFF);
  CHECK_EQ(r.lR1Mode, DRAW_OFF);
  CHECK_EQ(r.lR2Mode, DRAW_OFF);
  CHECK_EQ(r.lR3Mode, DRAW_OFF);

  const LeftAlarmFlags l = d.getLeftAlarms();
  CHECK_EQ(l.lUplinkAct, DRAW_OFF);
  CHECK_EQ(l.lTemp, DRAW_OFF);
  CHECK_EQ(l.lKeyErr, DRAW_OFF);
  CHECK_EQ(l.lOpErr, DRAW_OFF);
  // No active content view at boot.
  CHECK(d.getLeftView() == nullptr);
}

TEST(right_lcd_setters) {
  HmiData d;
  d.rSetnProg(1, DRAW_ON);
  const RightLcdData &r = d.getRightLcdData();
  CHECK_EQ(r.nProg, 1);
  CHECK_EQ(r.nProgMode, DRAW_ON);

  d.rSetnVerb(5, DRAW_BLINK);
  CHECK_EQ(d.getRightLcdData().nVerb, 5);
  CHECK_EQ(d.getRightLcdData().nVerbMode, DRAW_BLINK);

  d.rSetnNoun(9, DRAW_ERROR);
  CHECK_EQ(d.getRightLcdData().nNoun, 9);
  CHECK_EQ(d.getRightLcdData().nNounMode, DRAW_ERROR);

  d.rSetnR1(11, DRAW_ON);
  d.rSetnR2(22, DRAW_BLINK);
  d.rSetnR3(33, DRAW_ERROR);
  const RightLcdData &r2 = d.getRightLcdData();
  CHECK_EQ(r2.nR1, 11);
  CHECK_EQ(r2.nR1Mode, DRAW_ON);
  CHECK_EQ(r2.nR2, 22);
  CHECK_EQ(r2.nR2Mode, DRAW_BLINK);
  CHECK_EQ(r2.nR3, 33);
  CHECK_EQ(r2.nR3Mode, DRAW_ERROR);

  // Label setter touches only the label mode, not the value/mode.
  d.rSetVerbLabel(DRAW_ON);
  const RightLcdData &r3 = d.getRightLcdData();
  CHECK_EQ(r3.lVerbMode, DRAW_ON);
  CHECK_EQ(r3.nVerb, 5);
  CHECK_EQ(r3.nVerbMode, DRAW_BLINK);
}

TEST(left_lcd_alarm_setters) {
  HmiData d;
  d.lSetUplinkAct(DRAW_ON);
  d.lSetTemp(DRAW_BLINK);
  d.lSetKeyErr(DRAW_ERROR);
  d.lSetOpErr(DRAW_OFF);
  const LeftAlarmFlags l = d.getLeftAlarms();
  CHECK_EQ(l.lUplinkAct, DRAW_ON);
  CHECK_EQ(l.lTemp, DRAW_BLINK);
  CHECK_EQ(l.lKeyErr, DRAW_ERROR);
  CHECK_EQ(l.lOpErr, DRAW_OFF);
}

TEST(reset_right_lcd_data) {
  HmiData d;
  d.rSetnProg(1, DRAW_ON);
  d.rSetnVerb(5, DRAW_BLINK);
  d.rSetnNoun(9, DRAW_ERROR);
  d.rSetnR1(11, DRAW_ON);
  d.rSetnR2(22, DRAW_BLINK);
  d.rSetnR3(33, DRAW_ERROR);
  d.rSetProgLabel(DRAW_ON);
  d.rSetVerbLabel(DRAW_BLINK);
  d.rSetNounLabel(DRAW_ERROR);
  d.rSetCompActLabel(DRAW_ON);
  d.rSetR1Label(DRAW_BLINK);
  d.rSetR2Label(DRAW_ERROR);
  d.rSetR3Label(DRAW_ON);

  d.resetRightLcdData();
  const RightLcdData &r = d.getRightLcdData();
  CHECK_EQ(r.nProg, UNASIGNED);
  CHECK_EQ(r.nVerb, UNASIGNED);
  CHECK_EQ(r.nNoun, UNASIGNED);
  CHECK_EQ(r.nR1, UNASIGNED);
  CHECK_EQ(r.nR2, UNASIGNED);
  CHECK_EQ(r.nR3, UNASIGNED);
  CHECK_EQ(r.nProgMode, DRAW_OFF);
  CHECK_EQ(r.nVerbMode, DRAW_OFF);
  CHECK_EQ(r.nNounMode, DRAW_OFF);
  CHECK_EQ(r.nR1Mode, DRAW_OFF);
  CHECK_EQ(r.nR2Mode, DRAW_OFF);
  CHECK_EQ(r.nR3Mode, DRAW_OFF);
  CHECK_EQ(r.lCompActyMode, DRAW_OFF);
  CHECK_EQ(r.lProgMode, DRAW_OFF);
  CHECK_EQ(r.lVerbMode, DRAW_OFF);
  CHECK_EQ(r.lNounMode, DRAW_OFF);
  CHECK_EQ(r.lR1Mode, DRAW_OFF);
  CHECK_EQ(r.lR2Mode, DRAW_OFF);
  CHECK_EQ(r.lR3Mode, DRAW_OFF);
}

TEST(show_verb_list_basic) {
  HmiData d;
  d.showVerbList({"V00 LIST", "V21 TIME"});
  const VerbListView *v = d.leftViewAs<VerbListView>();
  CHECK(v != nullptr);
  CHECK_EQ(v->page, 0);
  CHECK_EQ(v->pages[0].size(), 2u);
  CHECK(v->pages[0][0] == "V00 LIST");
  CHECK(v->pages[0][1] == "V21 TIME");
}

TEST(show_verb_list_caps) {
  // More than 20 lines: only the first 20 are stored.
  HmiData d;
  std::vector<std::string> lines;
  for (int i = 0; i < 25; ++i) {
    lines.push_back("line-" + std::to_string(i));
  }
  d.showVerbList(lines);
  const VerbListView *v = d.leftViewAs<VerbListView>();
  CHECK(v != nullptr);
  CHECK_EQ(v->pages[0].size(), 20u);
  CHECK(v->pages[0][0] == "line-0");
  CHECK(v->pages[0][19] == "line-19");

  // A 40-char line is truncated to exactly 27 chars.
  HmiData d2;
  const std::string longLine(40, 'A');
  d2.showVerbList({longLine});
  const VerbListView *v2 = d2.leftViewAs<VerbListView>();
  CHECK(v2 != nullptr);
  CHECK_EQ(v2->pages[0].size(), 1u);
  CHECK_EQ(v2->pages[0][0].size(), 27u);
  CHECK(v2->pages[0][0] == std::string(27, 'A'));
}

TEST(show_verb_list_empty) {
  // Current behavior: even an empty list publishes a view (header-only page).
  HmiData d;
  d.showVerbList({});
  const VerbListView *v = d.leftViewAs<VerbListView>();
  CHECK(v != nullptr);
  CHECK_EQ(v->pages[0].size(), 0u);
}

TEST(show_verb_list_pages_and_headers) {
  HmiData d;
  std::array<std::string, 3> headers = {"KILL RUNNING PROGRAMS", "", ""};
  d.showVerbList({"P0"}, {"P1A", "P1B"}, {}, headers);
  const VerbListView *v = d.leftViewAs<VerbListView>();
  CHECK(v != nullptr);
  CHECK_EQ(v->pages[0].size(), 1u);
  CHECK_EQ(v->pages[1].size(), 2u);
  CHECK_EQ(v->pages[2].size(), 0u);
  CHECK(v->headers[0] == "KILL RUNNING PROGRAMS");
  CHECK(v->headers[1].empty());

  // A long header is truncated to exactly 27 chars.
  HmiData d2;
  std::array<std::string, 3> longHeaders = {std::string(40, 'H'), "", ""};
  d2.showVerbList({"A"}, {}, {}, longHeaders);
  const VerbListView *v2 = d2.leftViewAs<VerbListView>();
  CHECK(v2 != nullptr);
  CHECK_EQ(v2->headers[0].size(), 27u);
}

TEST(show_left_view_replaces_previous) {
  HmiData d;
  d.showVerbList({"OLD"});
  d.showVerbList({"NEW"});
  const VerbListView *v = d.leftViewAs<VerbListView>();
  CHECK(v != nullptr);
  CHECK_EQ(v->pages[0].size(), 1u);
  CHECK(v->pages[0][0] == "NEW");
}

TEST(clear_left_view_keeps_alarm_flags) {
  HmiData d;
  d.lSetUplinkAct(DRAW_ON);
  d.lSetTemp(DRAW_BLINK);
  d.lSetKeyErr(DRAW_ERROR);
  d.lSetOpErr(DRAW_ON);
  d.showVerbList({"A", "B"});

  d.clearLeftView();
  const LeftAlarmFlags l = d.getLeftAlarms();
  CHECK(d.getLeftView() == nullptr);
  CHECK_EQ(l.lUplinkAct, DRAW_ON);
  CHECK_EQ(l.lTemp, DRAW_BLINK);
  CHECK_EQ(l.lKeyErr, DRAW_ERROR);
  CHECK_EQ(l.lOpErr, DRAW_ON);
}

TEST(reset_left_lcd_data_clears_view) {
  HmiData d;
  d.lSetUplinkAct(DRAW_ON);
  d.lSetTemp(DRAW_BLINK);
  d.lSetKeyErr(DRAW_ERROR);
  d.lSetOpErr(DRAW_ON);
  d.showVerbList({"A", "B"});

  d.resetLeftLcdData();
  const LeftAlarmFlags l = d.getLeftAlarms();
  CHECK_EQ(l.lUplinkAct, DRAW_OFF);
  CHECK_EQ(l.lTemp, DRAW_OFF);
  CHECK_EQ(l.lKeyErr, DRAW_OFF);
  CHECK_EQ(l.lOpErr, DRAW_OFF);
  CHECK(d.getLeftView() == nullptr);
}

TEST(set_get_roundtrip) {
  HmiData d;

  RightLcdData r;
  r.nProg = 1;
  r.nVerb = 2;
  r.nNoun = 3;
  r.nR1 = 4;
  r.nR2 = 5;
  r.nR3 = 6;
  r.nProgMode = DRAW_ON;
  r.nVerbMode = DRAW_BLINK;
  r.nNounMode = DRAW_ERROR;
  r.nR1Mode = DRAW_OFF;
  r.nR2Mode = DRAW_ON;
  r.nR3Mode = DRAW_BLINK;
  r.lCompActyMode = DRAW_ERROR;
  r.lProgMode = DRAW_ON;
  r.lVerbMode = DRAW_BLINK;
  r.lNounMode = DRAW_ERROR;
  r.lR1Mode = DRAW_OFF;
  r.lR2Mode = DRAW_ON;
  r.lR3Mode = DRAW_BLINK;
  d.setRightLcdData(r);
  const RightLcdData &g = d.getRightLcdData();
  CHECK_EQ(g.nProg, 1);
  CHECK_EQ(g.nVerb, 2);
  CHECK_EQ(g.nNoun, 3);
  CHECK_EQ(g.nR1, 4);
  CHECK_EQ(g.nR2, 5);
  CHECK_EQ(g.nR3, 6);
  CHECK_EQ(g.nProgMode, DRAW_ON);
  CHECK_EQ(g.nVerbMode, DRAW_BLINK);
  CHECK_EQ(g.nNounMode, DRAW_ERROR);
  CHECK_EQ(g.nR1Mode, DRAW_OFF);
  CHECK_EQ(g.nR2Mode, DRAW_ON);
  CHECK_EQ(g.nR3Mode, DRAW_BLINK);
  CHECK_EQ(g.lCompActyMode, DRAW_ERROR);
  CHECK_EQ(g.lProgMode, DRAW_ON);
  CHECK_EQ(g.lVerbMode, DRAW_BLINK);
  CHECK_EQ(g.lNounMode, DRAW_ERROR);
  CHECK_EQ(g.lR1Mode, DRAW_OFF);
  CHECK_EQ(g.lR2Mode, DRAW_ON);
  CHECK_EQ(g.lR3Mode, DRAW_BLINK);

  LeftAlarmFlags l;
  l.lUplinkAct = DRAW_ON;
  l.lTemp = DRAW_BLINK;
  l.lKeyErr = DRAW_ERROR;
  l.lOpErr = DRAW_OFF;
  l.lShutdown = DRAW_OFF;
  l.lReboot = DRAW_OFF;
  d.setLeftAlarms(l);
  const LeftAlarmFlags gl = d.getLeftAlarms();
  CHECK_EQ(gl.lUplinkAct, DRAW_ON);
  CHECK_EQ(gl.lTemp, DRAW_BLINK);
  CHECK_EQ(gl.lKeyErr, DRAW_ERROR);
  CHECK_EQ(gl.lOpErr, DRAW_OFF);
}

TEST(r1_string_setter) {
  HmiData d;

  // String set/get round-trip; integer value is cleared.
  d.rSetStrR1("HELLO", DRAW_ON);
  const RightLcdData &r = d.getRightLcdData();
  CHECK(strcmp(r.sR1, "HELLO") == 0);
  CHECK_EQ(r.nR1Mode, DRAW_ON);
  CHECK_EQ(r.nR1, UNASIGNED);

  // String after int: last-set-wins, int value cleared.
  d.rSetnR1(42, DRAW_BLINK);
  d.rSetStrR1("AB", DRAW_ON);
  const RightLcdData &r2 = d.getRightLcdData();
  CHECK(strcmp(r2.sR1, "AB") == 0);
  CHECK_EQ(r2.nR1, UNASIGNED);

  // Int after string: last-set-wins, string cleared.
  d.rSetnR1(7, DRAW_BLINK);
  const RightLcdData &r3 = d.getRightLcdData();
  CHECK_EQ(r3.sR1[0], '\0');
  CHECK_EQ(r3.nR1, 7);

  // Truncation at 5 chars.
  d.rSetStrR1("123456789", DRAW_ON);
  const RightLcdData &r4 = d.getRightLcdData();
  CHECK(strcmp(r4.sR1, "12345") == 0);
  CHECK_EQ(strlen(r4.sR1), 5u);

  // Empty string leaves sR1 empty (renders as int path / DRAW_OFF).
  d.rSetStrR1("", DRAW_ON);
  const RightLcdData &r5 = d.getRightLcdData();
  CHECK_EQ(r5.sR1[0], '\0');

  // Reset clears both string and integer value.
  d.rSetStrR1("XY", DRAW_ON);
  d.resetRightLcdData();
  const RightLcdData &r6 = d.getRightLcdData();
  CHECK_EQ(r6.sR1[0], '\0');
  CHECK_EQ(r6.nR1, UNASIGNED);
  CHECK_EQ(r6.nR1Mode, DRAW_OFF);
}

TESTS_MAIN;
