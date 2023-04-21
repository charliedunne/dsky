#include <iostream>
#include "Frame.h"
#include <Logger.h>

Frame::Frame(unsigned int xPos, unsigned int yPos, unsigned int xSize, unsigned int ySize) {

}

Frame::~Frame() {
  
  std::cout << "Frame destructor" << std::endl;
}
