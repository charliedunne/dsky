#include <iostream>

#include "Hmi.h"
#include "SDL2/SDL.h"

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

Hmi::Hmi() {


  if (SDL_Init(SDL_INIT_VIDEO) !=0) {

    LogError << "SDL_Init Error: " << SDL_GetError() << std::endl;
  }

  window_ = SDL_CreateWindow("DSKY", 100, 100, 800, 480, SDL_WINDOW_SHOWN);
  if (window_ == NULL) {
    LogError << "SDL_CreateWindow()" << std::endl;
  }

  // Create Window surface
  screenSurface_ = SDL_GetWindowSurface( window_);
  if (screenSurface_ == NULL) {
    LogError << "SDL_GetWindowSurface()" << std::endl;
  } 

  //Fill the surface white
  if (SDL_FillRect( screenSurface_, NULL, SDL_MapRGB( screenSurface_->format, 0xFF, 0xFF, 0xFF ) ) != 0) {
    LogError << "SDL_FillRect: " << SDL_GetError() << std::endl;
  }

  //Update the surface
  if (SDL_UpdateWindowSurface( window_ ) != 0) {
    LogError << "SDL_UpdateWindowSurface: " << SDL_GetError() << std::endl;
  }

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

Hmi::~Hmi() {

  SDL_DestroyWindow(window_);
  SDL_Quit();
}
