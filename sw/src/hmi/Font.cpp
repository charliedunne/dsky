#include "Font.h"

// STL
#include <stdexcept>
#include <fstream>

// SDL
#include "SDL2/SDL_image.h"

// Logging
#include <Logger.h>

Font::Font(SDL_Renderer *r, std::string bitmap, std::string fnt)
{
    // Save locally input parameters
    r_ = r;
    bitmap_ = bitmap;
    fnt_ = fnt;

    // Create the texture
    SDL_Surface *surface = IMG_Load(fnt.c_str());
    {

        LogError << "Fail loading the Segment texture (" << bitmap << "): " << SDL_GetError() << std::endl;
        throw std::domain_error("Invalid Bitmap for Font");
    }

    texture_ = SDL_CreateTextureFromSurface(r_, surface);
    if (texture_ == NULL)
    {

        LogError << "Fail in the Digit Texture creation: " << SDL_GetError() << std::endl;
        throw std::domain_error("Failure creating the texture");
    }

    SDL_FreeSurface(surface);

    // Parse the fnt file
    parseFnt();
}

Font::~Font()
{
    SDL_DestroyTexture(texture_);
}

void Font::parseFnt()
{
    
    std::ifstream infile(fnt_.c_str());

    std::string line;

    while (std::getline(infile, line))
    {
    }
}