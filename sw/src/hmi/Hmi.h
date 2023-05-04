#ifndef HMI_H
#define HMI_H

#include <vector>
#include "Frame.h"
#include "FrameRightLcd.h"

#include "SDL2/SDL.h"

class Hmi {

 private:

  // Frames
  FrameRightLcd * rightLcd  = NULL;
  FrameRightLcd * centerLcd  = NULL;
  FrameRightLcd * leftLcd  = NULL;

  // Window properties
  int w_ = 0;
  int h_ = 0;

  // Window
  SDL_Window * win_ = NULL;

  // Renderer
  SDL_Renderer* ren_ = NULL;

public:

  // Default constructor
  Hmi(int w, int h);

  // Destructor
  ~Hmi();

  // Function for the display of the HMI
  void switchOn();

  // Function to switch off the display of the HMI 
  void switchOff();

  // Function to draw the HMI (it must be called every time it needs to be update)
  void render();

  void update(const char *);

  // Wait for event
  void wait();

  // @TODO Set functions for update values
};


#endif /* HMI_H */
