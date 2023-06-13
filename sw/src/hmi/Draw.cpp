// Provided interface
#include "Draw.h"

// STL
#include <iostream>

// Logging
#include "Logger.h"

Draw::Draw(const int x, const int y,
           const int period)
{
    x_ = x;
    y_ = y;
    blinkPeriod_ = period;

    // Default values
    mode_ = DRAW_OFF;
}

void Draw::draw()
{
    switch (mode_)
    {
    case DRAW_ON:
        drawOn();
        break;

    case DRAW_OFF:
        drawOff();
        break;

    case DRAW_ERROR:
        drawErr();
        break;

    case DRAW_BLINK:
        drawBlink();
        break;

    default:
        /** @TODO handle unexpected error */
        LogError << "Unexpected DRAW mode" << std::endl;
        break;
    }

    // Increase the frames counter
    frameCounter_ = (frameCounter_ + 1) % blinkPeriod_;
}

void Draw::drawBlink()
{
    if (frameCounter_ < blinkPeriod_ / 2)
    {
        drawOn();
    }
    else
    {
        drawOff();
    }
}

void Draw::drawErr()
{
    drawOff();
}

void Draw::setMode(const DrawMode mode, const int period)
{
    mode_ = mode;
    blinkPeriod_ = period;
}

void Draw::setBlinkingPeriod(const int period)
{
    blinkPeriod_ = period;
}