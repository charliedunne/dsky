#include "Font.h"

#include "SDL2/SDL.h"

#include <string>

int main()
{
    SDL_Renderer *r = nullptr;

    std::string bitmap = "../../../resources/fonts/Gorton-Condensed_32px.png";
    std::string fnt = "../../../resources/fonts/Gorton-Condensed_32px.fnt";

    Font(r, bitmap, fnt);
}