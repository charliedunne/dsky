#ifndef FRAMELEFTLCD_H
#define FRAMELEFTLCD_H


#include "SDL2/SDL.h"
#include "Frame.h"
#include "Color.h"
#include "Label.h"


class FrameLeftLcd : public Frame {

 private:

  const bool debug_ = true;

 public:

  // Constructor
  FrameLeftLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg); 

  // Destructor
  ~FrameLeftLcd();

  // Local implementation of Render
  void render();

};

#endif /* FRAMELEFTLCD_H */
