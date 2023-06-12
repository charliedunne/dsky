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
    uplinkAct = new AlarmLabel(r_, 0, 0,
                                      w_ / 2, (h_ / 2) / 3, std::string("UPLINK"));

    temp = new AlarmLabel(r_, w_ / 2 + 1, 0,
                                 w_ / 2, (h_ / 2) / 3, std::string("TEMP"));

    keyErr = new AlarmLabel(r_, 0, (h_ / 2) / 3 + 1,
                                   w_ / 2, (h_ / 2) / 3, std::string("KEY REL"));

    spare1 = new AlarmLabel(r_, w_ / 2 + 1, (h_ / 2) / 3 + 1,
                                   w_ / 2, (h_ / 2) / 3, std::string("NONE"));

    opError = new AlarmLabel(r_, 0, 2 * (h_ / 2) / 3 + 2,
                                    w_ / 2, (h_ / 2) / 3, std::string("OP ERR"));

    spare2 = new AlarmLabel(r_, w_ / 2 + 1, 2 * (h_ / 2) / 3 + 2,
                                   w_ / 2, (h_ / 2) / 3, std::string("NONE"));
}

FrameLeftLcd::~FrameLeftLcd()
{
    delete uplinkAct;
    delete temp;
    delete keyErr;
    delete spare1;
    delete opError;
    delete spare2;
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
    drawLines(Color(128, 128, 128), lineThickness_);

    // Draw Labels
    drawLabels();
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
        int y = (i * (heigh / numRows)) - i * thickness / 2;

        boxColor(r_,
                 x_, y,
                 x_ + w_, y + thickness,
                 c);
    }

    // Vertical Lines
    for (int i = 0; i < numCols + 1; ++i)
    {
        int x = i * width / numCols;

        boxColor(r_,
                 x, y_,
                 x + thickness, h_ / 2,
                 c);
    }
}

void FrameLeftLcd::drawLabels()
{
    uplinkAct->setMode(DRAW_ON);

    uplinkAct->draw();
    temp->draw();
    keyErr->draw();
    spare1->draw();
    opError->setMode(DRAW_BLINK);
    opError->draw();
    spare2->draw();
}