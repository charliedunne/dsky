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
  rightLcd_ = new FrameRightLcd(ren_, w_-330, 4, 330, h_, Color(0, 0, 0));
  leftLcd_ = new FrameLeftLcd(ren_, 2, 4, 335, h_-4, Color(0, 0, 0));
  centerLcd_ = new FrameCenterLcd(ren_, 331, 0, w_-331, h_-4, Color(0, 0, 0));

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
  centerLcd_->render();
  rightLcd_->render();
  leftLcd_->render();


  // Render the window
  SDL_RenderPresent(ren_);
}

void Hmi::update(HmiData &data) {

  // Set values
  rightLcd_->setProg(data.getRightLcdData().nProg, data.getRightLcdData().nProgMode);
  rightLcd_->setVerb(data.getRightLcdData().nVerb, data.getRightLcdData().nVerbMode);
  rightLcd_->setNoun(data.getRightLcdData().nNoun, data.getRightLcdData().nNounMode);
  rightLcd_->setR1(data.getRightLcdData().nR1, data.getRightLcdData().nR1Mode);
  rightLcd_->setR2(data.getRightLcdData().nR2, data.getRightLcdData().nR2Mode);
  rightLcd_->setR3(data.getRightLcdData().nR3, data.getRightLcdData().nR3Mode);

  // Set Numbers mode
  
  // Set flags
  rightLcd_->setProgStatus(data.getRightLcdData().lProgMode);
  rightLcd_->setVerbStatus(data.getRightLcdData().lVerbMode);
  rightLcd_->setNounStatus(data.getRightLcdData().lNounMode);
  rightLcd_->setCompActyStatus(data.getRightLcdData().lCompActyMode);
  rightLcd_->setR1Status(data.getRightLcdData().lR1Mode);
  rightLcd_->setR2Status(data.getRightLcdData().lR2Mode);
  rightLcd_->setR3Status(data.getRightLcdData().lR3Mode);


}
