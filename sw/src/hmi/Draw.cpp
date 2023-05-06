// Provided interface
#include "Draw.h"

Draw::Draw(SDL_Renderer *r, const int x, const int y,
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

    // Select the drawing (depending on the mode)
    if ((mode_ == DRAW_ON) ||
        ((mode_ == DRAW_BLINK) && (frameCounter_ < blinkPeriod_ / 2)))
    {
        drawOn();
    }
    else
    {
        drawOff();
    }

    // Increase the frames counter
    frameCounter_ = (frameCounter_ + 1) % blinkPeriod_;
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