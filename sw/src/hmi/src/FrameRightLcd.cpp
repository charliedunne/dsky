#include "FrameRightLcd.h"

#include <iostream>

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

// GFX
#include "GFX/SDL2_gfxPrimitives.h"

#ifndef FRAME_MARGIN
#define FRAME_MARGIN 20
#endif /* FRAME_MARGIN */

#ifndef INTERBLOCK_MARGIN
#define INTERBLOCK_MARGIN 10
#endif /* INTERBLOCK_MARGIN */

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

    int yPosTop = i * (h_*0.4)/2;

    // 3 top dots
    filledCircleColor(r_, xCenter, yPosTop + FRAME_MARGIN, dotsRadius, dotsColor);

    int yPosDown = i * (h_*0.5)/3 + h_*0.5;

    // 6 Side dots
    filledCircleColor(r_, xLeft, yPosDown, dotsRadius, dotsColor);
    filledCircleColor(r_, xRight, yPosDown, dotsRadius, dotsColor);
  }

  // Draw middle line
  boxColor(r_, 
           xLeft + FRAME_MARGIN, (h_*0.5) + dotsRadius/2,
           xRight - FRAME_MARGIN, (h_*0.5) + dotsRadius*1.5,
           lineColor);

}

void FrameRightLcd::progDigits() {

  stringColor(r_, 100, 100, "There was uppon a time", Color(255, 255, 255));
  
}


void FrameRightLcd::render() {

  frameBoundaries(Color(255, 255, 0));
  background();
  compActy(Color(20, 20, 20));
  //  progDigits();

}

