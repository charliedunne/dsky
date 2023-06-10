#include "Font.h"

// STL
#include <stdexcept>
#include <fstream>
#include <regex>

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
    SDL_Surface *surface = IMG_Load(bitmap.c_str());
    if (surface == NULL)
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
        charCoords_t coords;

        std::vector<std::string> elements;
        const char delim = ' ';

        char *token = std::strtok(const_cast<char *>(line.c_str()), &delim);
        while (token != nullptr)
        {
            std::string s = std::string(token);

            // ID
            if (std::regex_match(s, std::regex("^id=.*")))
            {
                coords.id = std::stoi(s.substr(3));
            }

            // X
            if (std::regex_match(s, std::regex("^x=.*")))
            {
                coords.x = std::stoi(s.substr(2));
            }

            // Y
            if (std::regex_match(s, std::regex("^y=.*")))
            {
                coords.y = std::stoi(s.substr(2));
            }

            // Width
            if (std::regex_match(s, std::regex("^width=.*")))
            {
                coords.w = std::stoi(s.substr(6));
            }

            // Height
            if (std::regex_match(s, std::regex("^height=.*")))
            {
                coords.h = std::stoi(s.substr(7));
            }

            // xoffset
            if (std::regex_match(s, std::regex("^xoffset=.*")))
            {
                coords.xoffset = std::stoi(s.substr(8));
            }

            // yoffset
            if (std::regex_match(s, std::regex("^yoffset=.*")))
            {
                coords.yoffset = std::stoi(s.substr(8));
            }

            // xadvance
            if (std::regex_match(s, std::regex("^xadvance=.*")))
            {
                coords.yoffset = std::stoi(s.substr(9));
            }

            // Add parameters to coords list
            coords_.push_back(coords);

            // Move to next sub-line element
            token = std::strtok(nullptr, &delim);
        }
    }
}