#include <iostream>
#include "Frame.h"
#include <Logger.h>


// Initialization of the object counter
unsigned int Frame::frameCounter_ = 0;

// Initialization of the global render clock
unsigned int Frame::renderFrame_ = 0;

/**
 * @brief Get the current value of the global render clock.
 *
 * Shared by all blinking elements so their phases stay synchronized.
 */
unsigned int Frame::frameCount() {
  return renderFrame_;
}

/**
 * @brief Advance the global render clock by one frame.
 *
 * Called once per Hmi::render() pass.
 */
void Frame::nextFrame() {
  renderFrame_++;
}

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
