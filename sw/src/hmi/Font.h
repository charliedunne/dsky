#ifndef _FONT_H_
#define _FONT_H_

// STL
#include <string>
#include <vector>

// Configuration
#include "config.h"

// SDL
#include "SDL2/SDL.h"

// Hmi
#include "Color.h"

typedef struct
{

    int id;
    int x;
    int y;
    int w;
    int h;
    int xoffset;
    int yoffset;
    int xadvance;

} charCoords_t;

class Font
{

private:

    /**
     * @brief Bitmap with all the glyphs in [PNG] format
     */
    std::string bitmap_;

    /** 
     * @brief Coordinate file as provided by https://snowb.org/
    */
    std::string fnt_;

    /**
     * @brief Pixel size (height) configured in the bitmap
     */
    int pixelHeight_;

    /**
     * @brief SPC Size 
     */
    int spcSize_;

    /**
     * @brief Vector of coordinates as extracted from fnt file 
     */
    std::vector<charCoords_t> coords_;

    // Renderer
    SDL_Renderer *r_ = NULL;

    // Texture with the gliphs
    SDL_Texture *texture_ = NULL;

    /**
     * @brief Text color
     */
    Color color_;
 
    void parseFnt();

    int getX(char c);
    int getY(char c);
    int getW(char c);
    int getH(char c);

public:
    /**
     * @brief Construct a new Font object
     *
     * @param r[in] Pointer to the renderer to use
     * @param bitmap[in] Path to the bitmap file [PNG]
     * @param fnt[in] Path to the coordinates text file
     */
    Font(SDL_Renderer *r, std::string bitmap, std::string fnt);
    virtual ~Font();

    void setColor(Color c);

    void drawText(std::string text, int x, int y);
};

#endif /* _FONT_H_ */