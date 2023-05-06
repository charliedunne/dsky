#include <iostream>

#include "Hmi.h"
#include "SDL2/SDL.h"

// Configuration
#include "config.h"

// Logging
#include <Logger.h>


Hmi::Hmi(int w, int h) {

  // Save window properties from constructor
  w_ = w;
  h_ = h;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) !=0) {
    LogError << "SDL_Init Error: " << SDL_GetError() << std::endl;
  }

  // Create the window
  win_ = SDL_CreateWindow("DSKY", 100, 100, w_, h_, SDL_WINDOW_FLAGS);
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

  // Create the frames
  rightLcd = new FrameRightLcd(ren_, w_-330, 0, 330, h_, Color(0, 0, 0));

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

  // Clear the screen
  if (SDL_RenderClear(ren_) != 0) {
    LogError << "SDL_RenderClear" << std::endl;
  }

  // Save all the Frames draw into buffer
  rightLcd->render();


  // Render the window
  SDL_RenderPresent(ren_);
}

void Hmi::update(HmiData &data) {
  rightLcd->setProg(data.prog);
}
