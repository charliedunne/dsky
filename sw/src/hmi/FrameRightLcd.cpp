#include "FrameRightLcd.h"

#include <iostream>

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

// GFX
#include "GFX/SDL2_gfxPrimitives.h"

// #include "Digits.h"
#include "Digit.h"

#ifndef FRAME_MARGIN
#define FRAME_MARGIN 20
#endif /* FRAME_MARGIN */

#ifndef INTERBLOCK_MARGIN
#define INTERBLOCK_MARGIN 10
#endif /* INTERBLOCK_MARGIN */

#define GHOST_COLOR_R 0x15
#define GHOST_COLOR_G 0x0B
#define GHOST_COLOR_B 0x0C
#define GHOST_COLOR_A 0xFF

#define LIVE_COLOR_R 0x00
#define LIVE_COLOR_G 0xCC
#define LIVE_COLOR_B 0x44
#define LIVE_COLOR_A 0xFF

void FrameRightLcd::drawFixedElements()
{

  // Set-up dot Radious (in px)
  const int dotsRadius = 4;

  // Draw dots
  int xCenter = x_ + (w_ / 2);
  int xLeft = x_ + FRAME_MARGIN * 1.5;
  int xRight = x_ + w_ - FRAME_MARGIN * 1.5;

  for (int i = 0; i < 3; ++i)
  {

    int yPosTop = i * (h_ * 0.39) / 2;

    // 3 top dots
    filledCircleColor(r_, xCenter, yPosTop + FRAME_MARGIN, dotsRadius, dotsColor_);

    int yPosDown = i * (h_ * 0.5) / 3 + h_ * 0.48;

    // 6 Side dots
    filledCircleColor(r_, xLeft, yPosDown, dotsRadius, dotsColor_);
    filledCircleColor(r_, xRight, yPosDown, dotsRadius, dotsColor_);
  }

  // Draw middle line
  boxColor(r_,
           xLeft + FRAME_MARGIN, (h_ * 0.48) + dotsRadius / 2,
           xRight - FRAME_MARGIN, (h_ * 0.48) + dotsRadius * 1.5,
           lineColor_);

  // Draw Ghost Numbers
  nProgGhost_->setValue("88");
  nProgGhost_->draw();

  nVerbGhost_->setValue("88");
  nVerbGhost_->draw();

  nNounGhost_->setValue("88");
  nNounGhost_->draw();

  nR1Ghost_->setValue("88888");
  nR1Ghost_->draw();

  nR2Ghost_->setValue("88888");
  nR2Ghost_->draw();

  nR3Ghost_->setValue("88888");
  nR3Ghost_->draw();

  // Draw ghost Lines
  for (int i = 0; i < 3; ++i)
  {
    int yPosDown = (i * (h_ * 0.5) / 3 + h_ * 0.48) -5;

    boxColor(r_,
             xLeft + FRAME_MARGIN, yPosDown + dotsRadius / 2,
             xRight - FRAME_MARGIN, yPosDown + dotsRadius * 1.5,
             ghostColor_);
  }
}

void FrameRightLcd::compActy(Color c)
{

  // Set Box Size
  int w = 115;
  int h = h_ * .40 / 2 - INTERBLOCK_MARGIN;

  boxColor(r_,
           x_ + FRAME_MARGIN, y_ + FRAME_MARGIN,
           x_ + FRAME_MARGIN + w, y_ + FRAME_MARGIN + h,
           Color(20, 20, 20, 0xFF));
}


void FrameRightLcd::progDigits()
{
  /**
   * @brief @TODO if needed in the future.
   *
   */

  stringColor(r_, 100, 100, "There was uppon a time", Color(255, 255, 255));
}

/**
 * @brief This function render the screen
 * @attention Note that this function will be called every frame to draw the
 * scene.
 */
void FrameRightLcd::render()
{
  /**
   * @brief Drawing procedure
   * @li Draw the Background color
   * @li (if DEBUG) draw the perimeter line
   * @li Draw the Fixed elements (dots and line)
   * @li Draw all the Ghost elements (Numbers and Labels)
   * @li Draw the live elements.
   */

  // Background Color
  boxColor(r_, x_, y_, x_ + w_, y_ + h_, bg_);

  // Frame perimeter
  if (debug_)
  {
    roundedRectangleColor(r_, x_, y_,
                          x_ + w_ - 1, y_ + h_ - 1,
                          M_PI * 8, Color(255, 255, 0));
  }

  // Draw Fixed elements
  drawFixedElements();
  // background();
  // compActy(Color(20, 20, 20));

  std::vector<bool> value = {1, 1, 1, 1, 1, 1, 1};

  nProg_->draw();
}

/**
 * @brief Construct the Right LCD Frame.
 *
 * @param r [in] Pointer to the Renderer
 * @param x [in] X Position
 * @param y [in] Y Position
 * @param w [in] Width
 * @param h [in] Height
 * @param bg [in] Background color
 */
FrameRightLcd::FrameRightLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg)
    : Frame(r, x, y, w, h, bg)
{

  /**
   * @brief This function will perform the following steps:
   * @li Save input parameters as in Frame constructor
   * @li Set-up the default colors
   * @li Create all the dynamic objects (digits, background digits, labels, lines, etc...)
   */

  // Colors
  ghostColor_ = Color(GHOST_COLOR_R, GHOST_COLOR_G, GHOST_COLOR_B, GHOST_COLOR_A);
  liveColor_ = Color(LIVE_COLOR_R, LIVE_COLOR_G, LIVE_COLOR_B, LIVE_COLOR_A);
  dotsColor_ = Color(0xF0, 0xF0, 0xF0, 0xFF);
  lineColor_ = Color(0xC0, 0xC0, 0xC0, 0xFF);

  // Create constants for positions
  const int nProg_x = 675;
  const int nProg_y = 40;
  const int nVerb_x = 500;
  const int nVerb_y = 150;

  const int nR1_x = 675 - (3 * S_WIDTH / 2 * S_FACTOR * N_SPACE_FACTOR);
  const int nR1_y = 235;
  const int nRxSep = 80;

  // Create Ghost numbers objects
  nProgGhost_ = new Number(r, 2, nProg_x, nProg_y, ghostColor_, false);
  nVerbGhost_ = new Number(r, 2, nVerb_x, nVerb_y, ghostColor_, false);
  nNounGhost_ = new Number(r, 2, nProg_x, nVerb_y, ghostColor_, false);

  nR1Ghost_ = new Number(r, 5, nR1_x, nR1_y, ghostColor_, false);
  nR2Ghost_ = new Number(r, 5, nR1_x, nR1_y + nRxSep, ghostColor_, false);
  nR3Ghost_ = new Number(r, 5, nR1_x, nR1_y + 2 * nRxSep, ghostColor_, false);

  // Create the Live number objects
  nProg_ = new Number(r, 2, nProg_x, nProg_y, liveColor_, false);
  nVerb_ = new Number(r, 2, nVerb_x, nVerb_y, liveColor_, false);
  nNoun_ = new Number(r, 2, nProg_x, nVerb_y, liveColor_, false);

  nR1_ = new Number(r, 5, nR1_x, nR1_y, liveColor_, false);
  nR2_ = new Number(r, 5, nR1_x, nR1_y + nRxSep, liveColor_, false);
  nR3_ = new Number(r, 5, nR1_x, nR1_y + 2 * nRxSep, liveColor_, false);
}

/**
 * @brief Destroy the Right LCD Frame
 */
FrameRightLcd::~FrameRightLcd()
{

  free(nProgGhost_);
  free(nVerbGhost_);
  free(nNounGhost_);
  free(nR1Ghost_);
  free(nR2Ghost_);
  free(nR3Ghost_);

  free(nProg_);
  free(nVerb_);
  free(nNoun_);
  free(nR1_);
  free(nR2_);
  free(nR3_);
}

void FrameRightLcd::setProg(const char *value)
{
  nProg_->setValue(value);
}
