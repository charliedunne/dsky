// Provided interface
#include "Draw.h"

// HMI Definitions
#include "Frame.h"

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
}

void Draw::drawBlink()
{
    // Use the global render clock (Frame::frameCount) so that all blinking
    // elements share the same phase reference and blink in sync with each other.
    const unsigned int phase = Frame::frameCount() % (unsigned int)blinkPeriod_;
    if (phase < (unsigned int)(blinkPeriod_ / 2))
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