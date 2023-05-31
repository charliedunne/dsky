#ifndef HMI_H
#define HMI_H

#include <vector>
#include "Frame.h"
#include "FrameRightLcd.h"
#include "FrameLeftLcd.h"

#include "SDL2/SDL.h"

#include "HmiDefinitions.h"

class Hmi {

 private:

  // Frames
  FrameRightLcd * rightLcd_  = NULL;
  FrameLeftLcd * centerLcd_  = NULL;
  FrameLeftLcd * leftLcd_  = NULL;

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

  // Function to draw the HMI (it must be called every time it needs to be update)
  void render();

  void update(HmiData&);

  // Wait for event
  void wait();

};


#endif /* HMI_H */
