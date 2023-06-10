#include "Font.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#include "config.h"

#include <string>
#include "Logger.h"

int main()
{

    //    IMG_Init(IMG_INIT_PNG);

    // Window
    SDL_Window *win_ = NULL;

    // Renderer
    SDL_Renderer *ren_ = NULL;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        LogError << "SDL_Init Error: " << SDL_GetError() << std::endl;
    }

    // Create the window
    win_ = SDL_CreateWindow("DSKY", 100, 100, 800, 640, SDL_WINDOW_SHOWN);
    if (win_ == NULL)
    {
        LogError << "SDL_CreateWindow()" << std::endl;
    }

    // Create the renderer
    ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren_ == NULL)
    {
        LogError << "SDL_CreateRenderer()" << std::endl;
    }

    std::string bitmap = "../../../resources/fonts/Gorton-Condensed_32px.png";
    // std::string bitmap = DIGIT_TXT_FILE;
    std::string fnt = "../../../resources/fonts/Gorton-Condensed_32px.fnt";

    Font(ren_, bitmap, fnt);
}