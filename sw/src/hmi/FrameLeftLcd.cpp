#include "FrameLeftLcd.h"

#include <iostream>

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

// GFX
#include "GFX/SDL2_gfxPrimitives.h"

FrameLeftLcd::FrameLeftLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg)
    : Frame(r, x, y, w, h, bg)
{

}

void FrameLeftLcd::render()
{

    // Background Color
    boxColor(r_, x_, y_, x_ + w_, y_ + h_, bg_);
}