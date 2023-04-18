#include <vector>
#include "Frame.h"

class Hmi {

 private:

  // Frames
  std::vector<Frame> frames_;


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
}
