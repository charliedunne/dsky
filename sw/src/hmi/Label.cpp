#include "Label.h"
#include "config.h"
#include "SDL2/SDL_image.h"
#include "GFX/SDL2_gfxPrimitives.h"
#include <Logger.h>

Label::Label(SDL_Renderer *r,
             const int x, const int y, const int w, const int h,
             const LabelType t) : Draw(x, y)
{
    // Control flag
    bool ctrlFlag = true;

    // Initial values for position and size
    r_ = r;
    w_ = w;
    h_ = h;
    bgColor_ = Color(LIVE_COLOR_R, LIVE_COLOR_G, LIVE_COLOR_B, LIVE_COLOR_A);
    fgColor_ = Color(0, 0, 0, 255);

    // Set the label type
    lType_ = t;

    // Create a surface from the PNG file
    SDL_Surface *surface = IMG_Load(LABEL_TXT_FILE);
    if (surface == NULL)
    {

        LogError << "Fail loading the Segment texture (" << LABEL_TXT_FILE << "): " << SDL_GetError() << std::endl;
        ctrlFlag = false;
    }

    // Create the Texture
    if (ctrlFlag)
    {
        labelTx_ = SDL_CreateTextureFromSurface(r_, surface);
        if (labelTx_ == NULL)
        {

            LogError << "Fail in the Digit Texture creation: " << SDL_GetError() << std::endl;
        }
    }

    // Release the surface resources
    SDL_FreeSurface(surface);
}

void drawLabel(SDL_Renderer *r, SDL_Texture *t, const int x, const int y, const int w, const int h, const Color c, const LabelType lType)
{
    // Create output rectange for drawing
    SDL_Rect dest = {x, y, w, h};

    // Create source from the Sprite
    SDL_Rect src = {0, 0, L_WIDTH, L_HEIGHT};

    // Draw the rectangle
    boxColor(r,
             x, y,
             x + w, y + h,
             c);

    if (lType != LABEL_LINE)
    {

        // Calculate Source based on the Label Type
        switch (lType)
        {
        case LABEL_VERB:
            src.y += L_HEIGHT;
            break;

        case LABEL_NOUN:
            src.y += 2 * L_HEIGHT;
            break;

        case LABEL_COMPACTY:
            src.y += 3 * L_HEIGHT;
            src.h += L_HEIGHT;
            break;

        case LABEL_PROG:
        default:
            break;
        }

        // Calculate the position of the texture to be center
        dest.x += (w - (L_WIDTH * L_FACTOR)) / 2;
        dest.w = L_WIDTH * L_FACTOR;
        dest.h = L_HEIGHT * L_FACTOR;

        if (lType == LABEL_COMPACTY)
        {
            dest.h *= 2;
            dest.y += (h - (2 * L_HEIGHT * L_FACTOR)) / 2;
        }
        else
        {
            dest.y += (h - (L_HEIGHT * L_FACTOR)) / 2;
        }

        // Draw the text
        SDL_SetTextureColorMod(t, 0, 0, 0);
        SDL_SetTextureAlphaMod(t, 255);
        SDL_RenderCopy(r, t, &src, &dest);
    }
}

void Label::drawOn()
{

    drawLabel(r_, labelTx_, x_, y_, w_, h_, fgColor_, lType_);
}

void Label::drawOff()
{
    drawLabel(r_, labelTx_, x_, y_, w_, h_, bgColor_, lType_);
}

void Label::setColor(const Color fgColor, const Color bgColor)
{
    fgColor_ = fgColor;
    bgColor_ = bgColor;
}
