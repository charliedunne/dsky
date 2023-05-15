#include "Sign.h"
#include "config.h"
#include "SDL2/SDL_image.h"
#include <Logger.h>

#include <iostream>

static std::vector<bool> getSegmentsFromChar(const char c)
{

    std::vector<bool> segments = {0, 0};

    switch (c)
    {

    case '+':
        segments[0] = 1;
        segments[1] = 1;
        break;

    case '-':
        segments[0] = 0;
        segments[1] = 1;
        break;

    default:
        segments[0] = 0;
        segments[1] = 0;
        break;
    }

    return segments;
}

Sign::Sign()
{

    // Save renderer
    r_ = NULL;

    // Initialize internal private members
    x_ = 0;
    y_ = 0;
    w_ = S_WIDTH / 2 * S_FACTOR;
    h_ = S_HEIGHT * S_FACTOR;
    fgColor_ = Color(LIVE_COLOR_R, LIVE_COLOR_G, LIVE_COLOR_B, LIVE_COLOR_A);
    bgColor_ = Color(GHOST_COLOR_R, GHOST_COLOR_G, GHOST_COLOR_B, GHOST_COLOR_A);
    glow_ = false;
}

Sign::Sign(SDL_Renderer *r) : Sign()
{

    Sign::initialize(r);
}

void Sign::initialize(SDL_Renderer *r)
{
    // Control flag
    bool ctrlFlag = true;

    // Save renderer
    r_ = r;

    // Create a surface from the BMP file
    SDL_Surface *surface = IMG_Load(SIGN_TXT_FILE);
    if (surface == NULL)
    {

        LogError << "Fail loading the Segment texture (" << SIGN_TXT_FILE << "): " << SDL_GetError() << std::endl;
        ctrlFlag = false;
    }

    SDL_Surface *surfaceGlow = IMG_Load(SIGN_GLOW_TXT_FILE);
    if (surface == NULL)
    {

        LogError << "Fail loading the Segment texture (" << SIGN_GLOW_TXT_FILE << "): " << SDL_GetError() << std::endl;
        ctrlFlag = false;
    }

    // Create the Texture
    if (ctrlFlag)
    {

        signTx_ = SDL_CreateTextureFromSurface(r_, surface);
        if (signTx_ == NULL)
        {

            LogError << "Fail in the Sign Texture creation: " << SDL_GetError() << std::endl;
        }

        signGlowTx_ = SDL_CreateTextureFromSurface(r_, surfaceGlow);
        if (signTx_ == NULL)
        {

            LogError << "Fail in the Glow Texture creation: " << SDL_GetError() << std::endl;
        }
    }

    // Release the surface resources
    SDL_FreeSurface(surface);
    SDL_FreeSurface(surfaceGlow);
}

Sign::~Sign()
{

    // Destroy the texture resources
    SDL_DestroyTexture(signTx_);
    SDL_DestroyTexture(signGlowTx_);
}

void Sign::setColor(const Color fgColor, const Color bgColor)
{
    fgColor_ = fgColor;
    bgColor_ = bgColor;
}

void Sign::setPosition(const int x, const int y)
{

    x_ = x;
    y_ = y;
}

void Sign::setSize(const int w, const int h)
{

    w_ = w;
    h_ = h;
}

void Sign::setGlow(const bool glow)
{
    glow_ = glow;
}

void Sign::draw(std::vector<bool> seg)
{

    // Create output rectange for drawing
    SDL_Rect dest = {x_, y_, w_, h_};

    // Create source from the Sprite
    SDL_Rect src = {0, 0, S_WIDTH / 2, S_HEIGHT};

    segments_ = seg;

    Color c;

    for (int i = 0; i < seg.size(); ++i)
    {

        if (seg[i])
        {
            c = fgColor_;
        }
        else
        {
            c = bgColor_;
        }

        // Set-up the src depending on the segment
        src.x = i * S_WIDTH;

        // Set the color
        SDL_SetTextureColorMod(signTx_, c.r(), c.g(), c.b());
        SDL_SetTextureAlphaMod(signTx_, c.a());
        SDL_SetTextureColorMod(signGlowTx_, c.r(), c.g(), c.b());
        SDL_SetTextureAlphaMod(signGlowTx_, c.a());

        // Draw it
        if (glow_)
        {
            SDL_RenderCopy(r_, signGlowTx_, &src, &dest);
        }
        SDL_RenderCopy(r_, signTx_, &src, &dest);
    }
}

void Sign::draw(const char c)
{

    Sign::draw(getSegmentsFromChar(c));
}
