#include <iostream>
#include "Frame.h"
#include <Logger.h>


// Initialization of the object counter
unsigned int Frame::frameCounter_ = 0;

Frame::Frame(SDL_Renderer *r, int x, int y, int w, int h, Color bg) {

  // Save reference to the renderer
  r_ = r;

  // Set the position, size and color internally
  x_ = x;
  y_ = y;
  w_ = w;
  h_ = h;
  bg_ = bg;

  // Get the frameId and increase the internal counter
  frameId_ = frameCounter_;
  frameCounter_++;

}

Frame::~Frame() {
  
  std::cout << "Frame destructor" << std::endl;
}
