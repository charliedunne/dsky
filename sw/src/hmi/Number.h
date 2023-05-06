#ifndef NUMBER_H
#define NUMBER_H

// Parent clases
#include "Draw.h"

// Required classes
#include "Digit.h"
#include "Color.h"

// Standard Library
#include <vector>
#include <string>

#define N_SPACE_FACTOR (1)

class Number : public Draw {

 private:

  // Array of digits
  std::vector<Digit *> digits_;

  // Value to print
  std::string value_;

  Digit *d_[5];

  Digit *a_ = NULL;

 public:

  Number(SDL_Renderer * r, const int digits, const int x, const int y, const Color c, const bool glow);
  
  void setValue(const std::string s);
  void setPosition(const int x, const int y);

  // Base class virtual functions
  void drawOn();
  void drawOff();

};

#endif /* NUMBER_H */
