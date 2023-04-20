#include <iostream>

#include "Hmi.h"
#include "SDL2/SDL.h"

// Logging
#include "BSlogger.hpp"

Hmi::Hmi() {
  
  if (SDL_Init(SDL_INIT_VIDEO) !=0) {
    
    std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
  }
  else {
    std::cout << "SDL Initialized correctly " << std::endl;
  }

  window_ = SDL_CreateWindow("DSKY", 100, 100, 800, 480, SDL_WINDOW_SHOWN);


  
  //Fill the surface white
  SDL_FillRect( screenSurface_, NULL, SDL_MapRGB( screenSurface_->format, 0xFF, 0xFF, 0xFF ) );
            
  //Update the surface
  SDL_UpdateWindowSurface( window_ );

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
