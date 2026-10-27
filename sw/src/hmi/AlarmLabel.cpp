#include "AlarmLabel.h"

// SDL
#include "GFX/SDL2_gfxPrimitives.h"

AlarmLabel::AlarmLabel(SDL_Renderer *r, const int x, const int y,
                       const int w, const int h, std::string text) : Draw(x, y)
{
    // Save internal parameters
    r_ = r;
    w_ = w;
    h_ = h;
    text_ = text;

    // Set default parameters
    fgColor_ = Color(LIVE_COLOR_R, LIVE_COLOR_G, LIVE_COLOR_B, LIVE_COLOR_A);
    bgColor_ = Color(GHOST_COLOR_R, GHOST_COLOR_G, GHOST_COLOR_B, GHOST_COLOR_A);

    fontBitmap_ = std::string(GORTON_FONT_BITMAP);
    fontFnt_ = std::string(GORTON_FONT_FNT);

    // Cache the font: constructing it per draw decodes the glyph bitmap from
    // disk on every frame (same pattern as FrameLeftLcd::verbListFont_)
    font_ = new Font(r, fontBitmap_, fontFnt_);
}

AlarmLabel::~AlarmLabel()
{
    delete font_;
}

void AlarmLabel::drawLabel(const Color c)
{
    Font &text = *font_;

    int textSizeW = 0;
    int textSizeH = 0;

    int textPosX = 0;
    int textPosY = 0;

    // Draw the rectangle
    boxColor(r_,
             x_, y_,
             x_ + w_, y_ + h_,
             c);

    /* Estimate the size of the text */
    text.estimateSize(text_, &textSizeW, &textSizeH);

    /* Calculate the position of the text to be centered in box */
    int centerX = x_ + w_/2;
    int centerY = y_ + h_/2;
    textPosX = centerX - textSizeW/2;
    textPosY = centerY - textSizeH/2;

    /* Draw the text */
    text.setColor(Color(0, 0, 0, 255));
    text.drawText(text_, textPosX, textPosY);
}

void AlarmLabel::setFgColor(const Color fgColor)
{
    fgColor_ = fgColor;
}

void AlarmLabel::setBgColor(const Color bgColor)
{
    bgColor_ = bgColor;
}

void AlarmLabel::drawOn()  
{
    drawLabel(fgColor_);
}

void AlarmLabel::drawOff()
{
    drawLabel(bgColor_);
}

void AlarmLabel::drawErr()
{
    drawLabel(Color(255, 50, 50));
}
