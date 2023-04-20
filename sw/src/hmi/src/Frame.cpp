#include <iostream>

#include "Frame.h"

Frame::Frame(unsigned int xPos, unsigned int yPos, unsigned int xSize, unsigned int ySize) {

  std::cout << "Frame constructor" << std::endl;
}

Frame::~Frame() {
  
  std::cout << "Frame destructor" << std::endl;
}
