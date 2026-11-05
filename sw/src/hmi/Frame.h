#ifndef FRAME_H
#define FRAME_H

#include "Color.h"
#include "SDL2/SDL.h"

class Frame {

private:

  // Frame counter
  static unsigned int frameCounter_;

  // Render clock, advanced once per Hmi::render() pass. Shared by all
  // blinking elements so their phases stay synchronized.
  static unsigned int renderFrame_;

protected:

  // Frame id
  unsigned int frameId_= 0;

  // Frame size and position
  int x_ = 0;
  int y_ = 0;
  int w_ = 0;
  int h_ = 0;

  // Background color
  Color bg_;

  // Reference to the window renderer
  SDL_Renderer *r_ = NULL;

public:

  // Constructor
  Frame(SDL_Renderer *r, int xPos, int yPos, int xSize, int ySize, Color bg);

  // Destructor
  ~Frame();

  // Render
  virtual void render() = 0;

  /**
   * @brief Global frame counter, advanced once per rendered frame.
   *
   * Shared by all blinking elements so their phases stay synchronized.
   */
  static unsigned int frameCount();

  /**
   * @brief Advance the global render clock by one frame.
   *
   * Called once per Hmi::render() pass so all blinking elements
   * share the same phase reference.
   */
  static void nextFrame();
};

#endif /* FRAME_H */
