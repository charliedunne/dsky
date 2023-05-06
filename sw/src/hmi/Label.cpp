#include "Label.h"
#include "config.h"
#include "SDL2/SDL_image.h"
#include "GFX/SDL2_gfxPrimitives.h"
#include <Logger.h>

Label::Label(SDL_Renderer *r,
             const int x, const int y, const int w, const int h,
             const LabelType t)
{
    // Control flag
    bool ctrlFlag = true;

    // Renderer
    r_ = r;

    // Initial values for position and size
    x_ = x;
    y_ = y;
    w_ = w;
    h_ = h;
    bgColor_ = Color(LIVE_COLOR_R, LIVE_COLOR_G, LIVE_COLOR_B, LIVE_COLOR_A);
    fgColor_ = Color(0, 0, 0, 255);

    // Set default status
    enabled_ = false;

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

void Label::draw()
{
    // Create output rectange for drawing
    SDL_Rect dest = {x_, y_, w_, h_};

    // Create source from the Sprite
    SDL_Rect src = {0, 0, L_WIDTH, L_HEIGHT};

    Color bgColor = bgColor_;

    // If disabled use Ghost color
    if (!enabled_)
    {
        bgColor.set(GHOST_COLOR_R, GHOST_COLOR_G, GHOST_COLOR_B, GHOST_COLOR_A);
    }

    // Draw the rectangle
    boxColor(r_,
             x_, y_,
             x_ + w_, y_ + h_,
             bgColor);

    // Calculate Source based on the Label Type
    switch (lType_)
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
    dest.x += (w_ - (L_WIDTH * L_FACTOR)) / 2;
    dest.w = L_WIDTH * L_FACTOR;
    dest.h = L_HEIGHT * L_FACTOR;

    if (lType_ == LABEL_COMPACTY)
    {
        dest.h *= 2;
        dest.y += (h_ - (2*L_HEIGHT * L_FACTOR)) / 2;
    }
    else
    {
        dest.y += (h_ - (L_HEIGHT * L_FACTOR)) / 2;
    }

    // Draw the text
    SDL_SetTextureColorMod(labelTx_, fgColor_.r(), fgColor_.g(), fgColor_.b());
    SDL_SetTextureAlphaMod(labelTx_, fgColor_.a());
    SDL_RenderCopy(r_, labelTx_, &src, &dest);
}

void Label::SwitchOn()
{

    enabled_ = true;
}

void Label::SwitchOff()
{

    enabled_ = false;
}
