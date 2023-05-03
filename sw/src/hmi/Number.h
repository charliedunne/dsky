#ifndef NUMBER_H
#define NUMBER_H

#include "Digit.h"
#include "Color.h"
#include <vector>
#include <string>

#define N_SPACE_FACTOR (1)

class Number {

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
  void draw();

  void setPosition(const int x, const int y);

};

#endif /* NUMBER_H */
