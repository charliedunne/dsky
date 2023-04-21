#include "FrameRightLcd.h"

#include <iostream>

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

// GFX
#include "GFX/SDL2_gfxPrimitives.h"

#ifndef FRAME_MARGIN
#define FRAME_MARGIN 10
#endif /* FRAME_MARGIN */

void FrameRightLcd::frameBoundaries(Color c) {

  SDL_Rect lcdBoundaries = {x_, y_, w_, h_};

  if (SDL_SetRenderDrawColor(r_, c.r(), c.g(), c.b(), c.a()) != 0) {
    LogError << "SDL_SetRenderDrawColor" << std::endl;
  }

  SDL_RenderDrawRect(r_, &lcdBoundaries);
}

void FrameRightLcd::compActy(Color c) {

  SDL_Rect compAct = {x_ + FRAME_MARGIN, 
    y_ + FRAME_MARGIN, 125, 125};

  if (SDL_SetRenderDrawColor(r_, c.r(), c.g() , c.b(), c.a()) != 0) {
    LogError << "SDL_SetRenderDrawColor" << std::endl;
  }

  SDL_RenderFillRect(r_, &compAct);

}

void FrameRightLcd::background() {

  circleRGBA(r_, 100, 100, 2* M_PI, 100, 255, 255, 255);
}

void FrameRightLcd::render() {


  frameBoundaries(Color(255, 255, 0));
  compActy(Color(20, 20, 20));
  background();

}


