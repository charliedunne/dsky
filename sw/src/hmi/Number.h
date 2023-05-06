#ifndef NUMBER_H
#define NUMBER_H

#include "Digit.h"
#include "Color.h"
#include <vector>
#include <string>

#define N_SPACE_FACTOR (1)

#define DEFAULT_BLINK_FRAMES 5

typedef enum {
  N_OFF = 0,
  N_ON = 1,
  N_BLINK = 2
} NumberMode;

class Number {

 private:

  // Array of digits
  std::vector<Digit *> digits_;

  // Value to print
  std::string value_;

  NumberMode nMode_ = N_ON;
  int blinkFrames_ = DEFAULT_BLINK_FRAMES;
  int frameCounter_ = 0;

  Digit *d_[5];

  Digit *a_ = NULL;

 public:

  Number(SDL_Renderer * r, const int digits, const int x, const int y, const Color c, const bool glow);
  
  void setValue(const std::string s);
  void draw();
  void setPosition(const int x, const int y);
  void setMode(const NumberMode mode, const int frames = DEFAULT_BLINK_FRAMES);

};

#endif /* NUMBER_H */
