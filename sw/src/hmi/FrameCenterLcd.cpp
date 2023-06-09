#include "FrameCenterLcd.h"

#include <iostream>

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

// GFX
#include "GFX/SDL2_gfxPrimitives.h"

FrameCenterLcd::FrameCenterLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg)
    : Frame(r, x, y, w, h, bg)
{
}

void FrameCenterLcd::render()
{
    // Background Color
    boxColor(r_, x_, y_, x_ + w_, y_ + h_, bg_);

    // Frame perimeter
    if (debug_)
    {
        roundedRectangleColor(r_, x_, y_,
                              x_ + w_ - 1, y_ + h_ - 1,
                              M_PI * 8, Color(255, 255, 0));
    }
}

