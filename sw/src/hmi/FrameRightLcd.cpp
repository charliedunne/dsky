#include "FrameRightLcd.h"

#include <iostream>

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

// GFX
#include "GFX/SDL2_gfxPrimitives.h"

//#include "Digits.h"
#include "Digit.h"


#ifndef FRAME_MARGIN
#define FRAME_MARGIN 20
#endif /* FRAME_MARGIN */

#ifndef INTERBLOCK_MARGIN
#define INTERBLOCK_MARGIN 10
#endif /* INTERBLOCK_MARGIN */

#define BG_COLOR_R 0x15
#define BG_COLOR_G 0x0B
#define BG_COLOR_B 0x0C
#define BG_COLOR_A 0xFF

#define FG_COLOR_R 0x00
#define FG_COLOR_G 0xCC
#define FG_COLOR_B 0x44
#define FG_COLOR_A 0xFF

void drawLinesBackground(SDL_Renderer *r, int x, int w, int h) {

  int xLeft = x + FRAME_MARGIN*1.5;
  int xRight = x + w - FRAME_MARGIN*1.5;

  // Set-up dot Radious
  const int dotsRadius = 4;

  const Color lineColor = Color(0xff, BG_COLOR_G, BG_COLOR_B, BG_COLOR_A);


  // Draw R1 line
  boxColor(r, 
           xLeft + FRAME_MARGIN, 222 + dotsRadius/2,
           xRight - FRAME_MARGIN, 222 + dotsRadius*1.5,
           lineColor);

  // Draw R2 line
  boxColor(r, 
           xLeft + FRAME_MARGIN, 222 + 85 + dotsRadius/2,
           xRight - FRAME_MARGIN, 222 + 85 + dotsRadius*1.5,
           lineColor);   

  // Draw R3 line
  boxColor(r, 
           xLeft + FRAME_MARGIN, 222 + 170 + dotsRadius/2,
           xRight - FRAME_MARGIN, 222 + 170 + dotsRadius*1.5,
           lineColor);                   

}


void FrameRightLcd::frameBoundaries(Color c) {

  roundedRectangleColor(r_, x_, y_, 
                        x_ + w_ -1, y_ + h_ - 1, 
                        M_PI*8, Color(255, 255, 0));
}

void FrameRightLcd::compActy(Color c) {

  // Set Box Size
  int w = 115;
  int h = h_*.40/2 - INTERBLOCK_MARGIN;

  boxColor(r_, 
           x_ + FRAME_MARGIN, y_ + FRAME_MARGIN,
           x_ + FRAME_MARGIN + w, y_ + FRAME_MARGIN + h,
           Color(20, 20, 20, 0xFF));
}

void FrameRightLcd::background() {

  // Set-up colors
  const Color dotsColor = Color(0xF0, 0xF0, 0xF0, 0xFF);
  const Color lineColor = Color(0xC0, 0xC0, 0xC0, 0xFF);

  // Set-up dot Radious
  const int dotsRadius = 4;

  // Draw dots
  int xCenter = x_ + (w_/2);
  int xLeft = x_ + FRAME_MARGIN*1.5;
  int xRight = x_ + w_ - FRAME_MARGIN*1.5;

  for (int i = 0; i < 3; ++i) {

    int yPosTop = i * (h_*0.39)/2;

    // 3 top dots
    filledCircleColor(r_, xCenter, yPosTop + FRAME_MARGIN, dotsRadius, dotsColor);
 
   int yPosDown = i * (h_*0.5)/2.8 + h_*0.48;

    // 6 Side dots
    filledCircleColor(r_, xLeft, yPosDown, dotsRadius, dotsColor);
    filledCircleColor(r_, xRight, yPosDown, dotsRadius, dotsColor);
  }

  // Draw middle line
  boxColor(r_, 
           xLeft + FRAME_MARGIN, (h_*0.48) + dotsRadius/2,
           xRight - FRAME_MARGIN, (h_*0.48) + dotsRadius*1.5,
           lineColor);

  // Draw digits
  nProgBg_->setValue("88");
  nProgBg_->draw();

  nVerbBg_->setValue("88");
  nVerbBg_->draw();

  nNounBg_->setValue("88");
  nNounBg_->draw();

  nR1Bg_->setValue("88888");
  nR1Bg_->draw();

  nR2Bg_->setValue("88888");
  nR2Bg_->draw();

  nR3Bg_->setValue("88888");
  nR3Bg_->draw();

  // Draw lines
  drawLinesBackground(r_, x_, w_, h_);

}


void FrameRightLcd::progDigits() {

  stringColor(r_, 100, 100, "There was uppon a time", Color(255, 255, 255));
}


void FrameRightLcd::render() {


  frameBoundaries(Color(255, 255, 0));
  background();
  compActy(Color(20, 20, 20));
  progDigits();

  std::vector<bool> value = {1, 1, 1, 1, 1, 1, 1};

  nProg_->setValue("03");
  nProg_->draw();

}

FrameRightLcd::FrameRightLcd(SDL_Renderer *r, int xPos, int yPos, int xSize, int ySize, Color bg) 
  : Frame(r, xPos, yPos, xSize, ySize, bg) {

  Color bgColor = Color(BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, BG_COLOR_A);
  Color fgColor = Color(FG_COLOR_R, FG_COLOR_G, FG_COLOR_B, FG_COLOR_A);

  // Create the numbers for background
  nProgBg_ = new Number(r, 2, 675, 40, bgColor, false);
  nVerbBg_ = new Number(r, 2, 500, 150, bgColor, false);
  nNounBg_ = new Number(r, 2, 675, 150, bgColor, false);

  nR1Bg_ = new Number(r, 5, 675-(3*S_WIDTH/2*S_FACTOR*N_SPACE_FACTOR), 240, bgColor, false);
  nR2Bg_ = new Number(r, 5, 675-(3*S_WIDTH/2*S_FACTOR*N_SPACE_FACTOR), 240+80, bgColor, false);
  nR3Bg_ = new Number(r, 5, 675-(3*S_WIDTH/2*S_FACTOR*N_SPACE_FACTOR), 240+80+80, bgColor, false);

  // Create the numbers for background
  nProg_ = new Number(r, 2, 675, 40, fgColor, true);
  nVerb_ = new Number(r, 2, 500, 150, fgColor, true);
  nNoun_ = new Number(r, 2, 675, 150, fgColor, true);

  nR1_ = new Number(r, 5, 675-(3*S_WIDTH/2*S_FACTOR*N_SPACE_FACTOR), 240, fgColor, true);
  nR2_ = new Number(r, 5, 675-(3*S_WIDTH/2*S_FACTOR*N_SPACE_FACTOR), 240+80, fgColor, true);
  nR3_ = new Number(r, 5, 675-(3*S_WIDTH/2*S_FACTOR*N_SPACE_FACTOR), 240+80+80, fgColor, true);

}

FrameRightLcd::~FrameRightLcd() {

 free(nProgBg_);
 free(nVerbBg_);
 free(nNounBg_);
 free(nR1Bg_);
 free(nR2Bg_);
 free(nR3Bg_);

 free(nProg_);
 free(nVerb_);
 free(nNoun_);
 free(nR1_);
 free(nR2_);
 free(nR3_);
 
}
