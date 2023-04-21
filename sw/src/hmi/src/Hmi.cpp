#include <iostream>

#include "Hmi.h"
#include "SDL2/SDL.h"

// Configuration
#include "config.h"

// Logging
#include <Logger.h>


Hmi::Hmi() {

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) !=0) {
    LogError << "SDL_Init Error: " << SDL_GetError() << std::endl;
  }

  // Create the window
  win_ = SDL_CreateWindow("DSKY", 100, 100, 800, 480, SDL_WINDOW_SHOWN);
  if (win_ == NULL) {
    LogError << "SDL_CreateWindow()" << std::endl;
  }

  // Create the renderer
  ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
  if (ren_ == NULL) {
    LogError << "SDL_CreateRenderer()" << std::endl;
  } 

  // Disable cursor
  SDL_ShowCursor(SDL_DISABLE);

}

Hmi::~Hmi() {

  SDL_DestroyWindow(win_);
  SDL_Quit();
}

void Hmi::wait() {

  //Hack to get window to stay up
  SDL_Event e; 
  bool quit = false; 
  while( quit == false ){ 
    while( SDL_PollEvent( &e ) ) { 
      if( e.type == SDL_QUIT ) 
        quit = true; 
    } 
  }
}


void Hmi::render() {

  SDL_Rect lcdLeft;
  lcdLeft.x = 0;
  lcdLeft.y = 0;
  lcdLeft.w = 350;
  lcdLeft.h = 480;

  SDL_Rect lcdRight = {450, 0, 350, 480};


  if (SDL_RenderClear(ren_) != 0) {
    LogError << "SDL_RenderClear" << std::endl;
  }

  if (SDL_SetRenderDrawColor(ren_, 255, 255, 255, SDL_ALPHA_OPAQUE) != 0) {
    LogError << "SDL_SetRenderDrawColor" << std::endl;
  }

  SDL_RenderFillRect(ren_, &lcdLeft);

  if (SDL_SetRenderDrawColor(ren_, 0, 255, 255, SDL_ALPHA_OPAQUE) != 0) {
    LogError << "SDL_SetRenderDrawColor" << std::endl;
  }

  SDL_RenderFillRect(ren_, &lcdRight);


  SDL_RenderPresent(ren_);
}
