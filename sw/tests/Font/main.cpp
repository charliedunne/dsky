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

    std::string bitmap = GORTON_FONT_BITMAP;
    // std::string bitmap = DIGIT_TXT_FILE;
    std::string fnt = GORTON_FONT_FNT;

    Font text = Font(ren_, bitmap, fnt);

    // Clear the screen
    if (SDL_RenderClear(ren_) != 0)
    {
        LogError << "SDL_RenderClear" << std::endl;
    }

    text.drawText(std::string("HELLO WORLD\nMY FRIENDS"), 100, 100);

    // Render the window
    SDL_RenderPresent(ren_);

    SDL_Delay(10000);
}