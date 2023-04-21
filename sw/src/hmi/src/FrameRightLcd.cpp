#include "FrameRightLcd.h"

#include <iostream>

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

void renderFrameBoundaries(SDL_Renderer *r, int x, int y, int w, int h, Color bg) {

  SDL_Rect lcdBoundaries = {x, y, w, h};

  if (SDL_RenderClear(r) != 0) {
    LogError << "SDL_RenderClear" << std::endl;
  }

  if (SDL_SetRenderDrawColor(r, bg.r(), bg.g() , bg.b(), bg.a()) != 0) {
    LogError << "SDL_SetRenderDrawColor" << std::endl;
  }

  SDL_RenderDrawRect(r, &lcdBoundaries);


}

void FrameRightLcd::render() {


  // Print the Frame boundaries
  renderFrameBoundaries(r_, xPos_, yPos_, xSize_, ySize_, bg_);
}
