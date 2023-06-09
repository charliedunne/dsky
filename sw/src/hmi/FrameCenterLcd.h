#ifndef _FRAMECENTERLCD_H_
#define _FRAMECENTERLCD_H_

#include "SDL2/SDL.h"
#include "Frame.h"
#include "Color.h"
#include "Label.h"

class FrameCenterLcd : public Frame
{

private:
  const bool debug_ = false;

public:
  // Constructor
  FrameCenterLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg);

  // Destructor
  ~FrameCenterLcd();

  // Local implementation of Render
  void render();

};

#endif /* _FRAMECENTERLCD_H_ */
