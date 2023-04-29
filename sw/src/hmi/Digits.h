#ifndef DIGITS_H
#define DIGITS_H

#include <string>

#include "Color.h"
#include "SDL2/SDL_ttf.h"

typedef enum {
  DIGITS_ON,
  DIGITS_OFF
} DigitStatus;


class Digits {

 private:

  // Status
  DigitStatus status_;

  // TTF font
  TTF_Font *font_;

  // Size in (pt)
  int ptsize;

  // Number of digits 
  unsigned int digits_;

  // String to show
  std::string value_;

  // Position
  int x_;
  int y_;

  // Boolean flag to check TTF Initialization
  static bool ttfInitialized_;
  static int instanceCounter_;

  // Color
  Color fgColor_;
  Color bgColor_;
  Color fgColorDisable_;

 public:

  // Constructor
  Digits();

  // Destructor
  ~Digits();

  // Enable Label
  void switchOn();

  // Disable Label
  void switchOff();

  // Blink Label
  void Blink(unsigned int freq);

  // Set Value
  void setShortValue(int value);
  void setLongValue(int value);
  void setValue(std::string value);

  // Set Position
  void setPosition(int x, int y);

  // Draw the digit
  void draw(SDL_Renderer *ren);

};

#endif /* DIGITS_H */
