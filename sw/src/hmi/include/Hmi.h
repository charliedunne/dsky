#include <vector>
#include "Frame.h"

#include "SDL2/SDL.h"

class Hmi {

 private:

  // Frames
  std::vector<Frame> frames_;

  // Window
  SDL_Window * win_ = NULL;

  // Renderer
  SDL_Renderer* ren_ = NULL;

public:

  // Default constructor
  Hmi();

  // Destructor
  ~Hmi();

  // Function for the display of the HMI
  void switchOn();

  // Function to switch off the display of the HMI 
  void switchOff();

  // Function to draw the HMI (it must be called every time it needs to be update)
  void render();

  // Wait for event
  void wait();

  // @TODO Set functions for update values
};
