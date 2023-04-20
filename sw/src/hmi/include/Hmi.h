#include <vector>
#include "Frame.h"

#include "SDL2/SDL.h"

class Hmi {

 private:

  // Frames
  std::vector<Frame> frames_;

  // Window
  SDL_Window * window_ = NULL;

  //The surface contained by the window
  SDL_Surface* screenSurface_ = NULL;

public:

  // Default constructor
  Hmi();

  // Destructor
  ~Hmi();

  // Function for the display of the HMI
  void SwitchOn();

  // Function to switch off the display of the HMI 
  void SwitchOff();

  // Function to draw the HMI (it must be called every time it needs to be update)
  void Render();

  // @TODO Set functions for update values
};
