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

    // Frame perimeter
    if (debug_)
    {
        roundedRectangleColor(r_, x_, y_,
                              x_ + w_ - 1, y_ + h_ - 1,
                              M_PI * 8, Color(255, 255, 0));
    }

    // Draw the separation lines
    drawLines(Color(128, 128, 128), 15);
}

void FrameLeftLcd::drawLines(Color c, int thickness)
{
    int width = w_ - thickness;
    int heigh = h_ / 2 - thickness;

    int numRows = 3;
    int numCols = 2;

    // Horizontal lines
    for (int i = 0; i < numRows + 1; ++i)
    {
        int y = i * heigh / (numRows);

        boxColor(r_,
                 x_, y,
                 x_ + w_, y + thickness,
                 c);
    }

    // Vertical Lines
    for (int i = 0; i < numCols + 1; ++i)
    {
        int x = i * width/numCols;

        boxColor(r_,
                 x, y_,
                 x + thickness, h_ / 2,
                 c);
    }
}