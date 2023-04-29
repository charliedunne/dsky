#ifndef DIGIT_H
#define DIGIT_H

#include "Color.h"
#include "SDL2/SDL.h"

typedef enum {

  D_0,
  D_1,
  D_2,
  D_3,
  D_4,
  D_5,
  D_6,
  D_7,
  D_8,
  D_9,
  C_A,
  C_b,
  C_C,
  C_d,
  C_E,
  C_F,
  C_G,
  C_H,
  C_J,
  C_L,
  C_P,
  C_r,
  C_U,
  S_0,
  S_1,
  S_2,
  S_3,
  S_4,
  S_5,
  S_6,
  S_7
} DigitChar_e;

class Digit {

 private:

  // Renderer
  SDL_Renderer r_ = NULL;
  
  // Texture with the gliphs
  SDL_Texture *digitTx_ = NULL;

  // Position and size
  int x_;
  int y_;
  int w_;
  int h_;

  // Color
  Color color_;

  // Internal signal for every segment
  bool segments_[7];

 public:

  Digit(SDL_Renderer r);
  ~Digit();

  void setColor(const Color color);
  void setPosition(const int x, const int y);
  void setSize(const int w, const int h);

  void draw(const DigitChar_e d) const;

}

#endif /* DIGIT_H */
