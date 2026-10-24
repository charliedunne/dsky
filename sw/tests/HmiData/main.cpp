#include "../common/test.h"
#include "HmiData.h"
#include <cstring>
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

  const LeftLcdData &l = d.getLeftLcdData();
  CHECK_EQ(l.lUplinkAct, DRAW_OFF);
  CHECK_EQ(l.lTemp, DRAW_OFF);
  CHECK_EQ(l.lKeyErr, DRAW_OFF);
  CHECK_EQ(l.lOpErr, DRAW_OFF);
  CHECK_EQ(l.lVerbList, DRAW_OFF);
  CHECK_EQ(l.lVerbListCount, 0);
  // NOTE: lVerbListLines is never zero-initialized; do not read it here.
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
  const LeftLcdData &l = d.getLeftLcdData();
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

TEST(l_set_verb_list_basic) {
  HmiData d;
  std::vector<std::string> lines = {"V00 LIST", "V21 TIME"};
  d.lSetVerbList(lines);
  const LeftLcdData &l = d.getLeftLcdData();
  CHECK_EQ(l.lVerbList, DRAW_ON);
  CHECK_EQ(l.lVerbListCount, 2);
  CHECK(strcmp(l.lVerbListLines[0], "V00 LIST") == 0);
  CHECK(strcmp(l.lVerbListLines[1], "V21 TIME") == 0);
}

TEST(l_set_verb_list_caps) {
  // More than 20 lines: only the first 20 are stored.
  HmiData d;
  std::vector<std::string> lines;
  for (int i = 0; i < 25; ++i) {
    lines.push_back("line-" + std::to_string(i));
  }
  d.lSetVerbList(lines);
  const LeftLcdData &l = d.getLeftLcdData();
  CHECK_EQ(l.lVerbListCount, 20);
  CHECK(strcmp(l.lVerbListLines[0], "line-0") == 0);
  CHECK(strcmp(l.lVerbListLines[19], "line-19") == 0);

  // A 40-char line is truncated to exactly 27 chars + NUL.
  HmiData d2;
  const std::string longLine(40, 'A');
  d2.lSetVerbList({longLine});
  const LeftLcdData &l2 = d2.getLeftLcdData();
  CHECK_EQ(l2.lVerbListCount, 1);
  CHECK_EQ(std::strlen(l2.lVerbListLines[0]), 27u);
  CHECK(l2.lVerbListLines[0][27] == '\0');
  CHECK(strcmp(l2.lVerbListLines[0], std::string(27, 'A').c_str()) == 0);
}

TEST(l_set_verb_list_empty) {
  // Current behavior: even an empty list turns the menu ON with count 0.
  HmiData d;
  d.lSetVerbList({});
  const LeftLcdData &l = d.getLeftLcdData();
  CHECK_EQ(l.lVerbListCount, 0);
  CHECK_EQ(l.lVerbList, DRAW_ON);
}

TEST(clear_verb_list_keeps_alarm_flags) {
  HmiData d;
  d.lSetUplinkAct(DRAW_ON);
  d.lSetTemp(DRAW_BLINK);
  d.lSetKeyErr(DRAW_ERROR);
  d.lSetOpErr(DRAW_ON);
  d.lSetVerbList({"A", "B"});

  d.clearVerbList();
  const LeftLcdData &l = d.getLeftLcdData();
  CHECK_EQ(l.lVerbList, DRAW_OFF);
  CHECK_EQ(l.lVerbListCount, 0);
  CHECK_EQ(l.lUplinkAct, DRAW_ON);
  CHECK_EQ(l.lTemp, DRAW_BLINK);
  CHECK_EQ(l.lKeyErr, DRAW_ERROR);
  CHECK_EQ(l.lOpErr, DRAW_ON);
}

TEST(reset_left_lcd_data_clears_verb_list) {
  HmiData d;
  d.lSetUplinkAct(DRAW_ON);
  d.lSetTemp(DRAW_BLINK);
  d.lSetKeyErr(DRAW_ERROR);
  d.lSetOpErr(DRAW_ON);
  d.lSetVerbList({"A", "B"});

  d.resetLeftLcdData();
  const LeftLcdData &l = d.getLeftLcdData();
  CHECK_EQ(l.lUplinkAct, DRAW_OFF);
  CHECK_EQ(l.lTemp, DRAW_OFF);
  CHECK_EQ(l.lKeyErr, DRAW_OFF);
  CHECK_EQ(l.lOpErr, DRAW_OFF);
  CHECK_EQ(l.lVerbList, DRAW_OFF);
  CHECK_EQ(l.lVerbListCount, 0);
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

  LeftLcdData l;
  l.lUplinkAct = DRAW_ON;
  l.lTemp = DRAW_BLINK;
  l.lKeyErr = DRAW_ERROR;
  l.lOpErr = DRAW_OFF;
  l.lVerbList = DRAW_ON;
  std::strcpy(l.lVerbListLines[0], "RT LINE");
  l.lVerbListCount = 1;
  d.setLeftLcdData(l);
  const LeftLcdData &gl = d.getLeftLcdData();
  CHECK_EQ(gl.lUplinkAct, DRAW_ON);
  CHECK_EQ(gl.lTemp, DRAW_BLINK);
  CHECK_EQ(gl.lKeyErr, DRAW_ERROR);
  CHECK_EQ(gl.lOpErr, DRAW_OFF);
  CHECK_EQ(gl.lVerbList, DRAW_ON);
  CHECK(strcmp(gl.lVerbListLines[0], "RT LINE") == 0);
  CHECK_EQ(gl.lVerbListCount, 1);
}

TESTS_MAIN;
