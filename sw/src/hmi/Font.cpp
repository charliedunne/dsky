#include "Font.h"

// STL
#include <stdexcept>
#include <fstream>
#include <regex>
#include <algorithm>

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

    // Default color
    color_ = Color(255, 255, 255, 255);

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

    // FIle to parse
    std::ifstream infile(fnt_.c_str());

    // Container for line
    std::string line;

    while (std::getline(infile, line))
    {
        charCoords_t coords;

        std::vector<std::string> elements;
        const char delim = ' ';

        // Report that element was found
        bool fId = false;
        bool fX = false;
        bool fY = false;
        bool fW = false;
        bool fH = false;
        bool fXoffset = false;
        bool fYoffset = false;
        bool fXadvance = false;

        char *token = std::strtok(const_cast<char *>(line.c_str()), &delim);
        while (token != nullptr)
        {
            std::string s = std::string(token);

            // ID
            if (std::regex_match(s, std::regex("^id=.*")))
            {
                coords.id = std::stoi(s.substr(3));
                fId = true;
            }

            // X
            if (std::regex_match(s, std::regex("^x=.*")))
            {
                coords.x = std::stoi(s.substr(2));
                fX = true;
            }

            // Y
            if (std::regex_match(s, std::regex("^y=.*")))
            {
                coords.y = std::stoi(s.substr(2));
                fY = true;
            }

            // Width
            if (std::regex_match(s, std::regex("^width=.*")))
            {
                coords.w = std::stoi(s.substr(6));
                fW = true;
            }

            // Height
            if (std::regex_match(s, std::regex("^height=.*")))
            {
                coords.h = std::stoi(s.substr(7));
                fH = true;
            }

            // xoffset
            if (std::regex_match(s, std::regex("^xoffset=.*")))
            {
                coords.xoffset = std::stoi(s.substr(8));
                fXoffset = true;
            }

            // yoffset
            if (std::regex_match(s, std::regex("^yoffset=.*")))
            {
                coords.yoffset = std::stoi(s.substr(8));
                fYoffset = true;
            }

            // xadvance
            if (std::regex_match(s, std::regex("^xadvance=.*")))
            {
                coords.yoffset = std::stoi(s.substr(9));
                fXadvance = true;
            }

            // Add parameters to coords list
            if (fId & fX & fY & fW & fH & fXoffset & fYoffset & fXadvance)
            {
                coords_.push_back(coords);

                // Avoid to include copies if there are more information not
                // parsed in the line
                fId = false;
            }

            // Move to next sub-line element
            token = std::strtok(nullptr, &delim);
        }
    }

    // Get the pixelHeight
    auto max = std::max_element(coords_.begin(), coords_.end(),
                                [](const charCoords_t &p1, const charCoords_t &p2)
                                {
                                    return p1.h < p2.h;
                                });

    pixelHeight_ = (int)max->h;
    spcSize_ = (int)((pixelHeight_ / 3));

    // std::cout << "pixeHeight_ = " << pixelHeight_ << ", spcSize_ = " << spcSize_ << std::endl;
    // std::cout << "Number of glyphs: " << coords_.size() << std::endl;
}

void Font::setColor(Color c)
{
    color_ = c;
}

void Font::drawText(std::string text, int x, int y)
{
    int xPos = x;
    int yPos = y;

    // for (int i = 0; i < coords_.size(); ++i)
    // {
    //     std::cout << "id = " << coords_[i].id;
    //     std::cout << ", x = " << coords_[i].x;
    //     std::cout << ", y = " << coords_[i].y;
    //     std::cout << ", w = " << coords_[i].w;
    //     std::cout << ", h = " << coords_[i].h << std::endl;
    // }

    for (int i = 0; i < text.size(); ++i)
    {
        // Get the character coords
        int xCoord = getX(text[i]);
        int yCoord = getY(text[i]);
        int width = getW(text[i]);
        int height = getH(text[i]);

        // std::cout << "xCoord = " << xCoord;
        // std::cout << ", yCoord = " << yCoord;
        // std::cout << ", width = " << width;
        // std::cout << ", height = " << height << std::endl;

        if ((xCoord != -1) && (yCoord != -1) && (width != -1) && (height != -1))
        {
            // Destination coordenates
            SDL_Rect dest = {xPos, yPos, width, height};

            /* Special case for the SPC */
            if (text[i] == 32)
            {
                dest.w = spcSize_;
                dest.h = pixelHeight_;
            }

            // Source coordinates
            SDL_Rect src = {xCoord, yCoord, width, height};

            // Set the color
            SDL_SetTextureColorMod(texture_, color_.r(), color_.g(), color_.b());
            SDL_SetTextureAlphaMod(texture_, color_.a());

            SDL_RenderCopy(r_, texture_, &src, &dest);

            // Update position for next character
            xPos += dest.w;
        }

        if (text[i] == '\n')
        {
            xPos = x;
            yPos += pixelHeight_;
        }
    }
}

void Font::drawText(std::string text, int x, int y, Color c)
{
    color_ = c;
    drawText(text, x, y);
}

int Font::getX(char c)
{
    for (int i = 0; i < coords_.size(); ++i)
    {
        if (coords_[i].id == c)
        {
            return coords_[i].x;
        }

        /** @TODO Special cases '\n', ' ', etc... */
    }

    return -1;
}

int Font::getY(char c)
{
    for (int i = 0; i < coords_.size(); ++i)
    {
        if (coords_[i].id == c)
        {
            return coords_[i].y;
        }

        /** @TODO Special cases '\n', ' ', etc... */
    }

    return -1;
}

int Font::getH(char c)
{
    for (int i = 0; i < coords_.size(); ++i)
    {
        if (coords_[i].id == c)
        {
            return coords_[i].h;
        }

        /** @TODO Special cases '\n', ' ', etc... */
    }

    return -1;
}

int Font::getW(char c)
{
    for (int i = 0; i < coords_.size(); ++i)
    {
        if (coords_[i].id == c)
        {
            return coords_[i].w;
        }

        /** @TODO Special cases '\n', ' ', etc... */
    }

    return -1;
}

void Font::estimateSize(std::string text, int *w, int *h)
{
    /* Initial size */
    *w = 0;
    *h = pixelHeight_;

    for (int i = 0; i < text.size(); ++i)
    {
        int width = getW(text[i]);
        int height = getH(text[i]);

        if ((width != -1) && (height != -1))
        {
            if (text[i] == 32)
            {
                *w += spcSize_;
            }
            else if (text[i] == '\n')
            {
                *h += pixelHeight_;
            }
            else
            {
                *w += width;
            }
        }
    }
}