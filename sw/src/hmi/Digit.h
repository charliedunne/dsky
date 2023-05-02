#ifndef DIGIT_H
#define DIGIT_H

#include <vector>

#include "Color.h"
#include "SDL2/SDL.h"

#define N_SEGMENTS (7)
#define S_WIDTH 280
#define S_HEIGHT 200

class Digit {

 private:

  // Renderer
  SDL_Renderer * r_ = NULL;
  
  // Texture with the gliphs
  SDL_Texture *digitTx_ = NULL;
  SDL_Texture *digitGlowTx_ = NULL;

  // Position and size
  int x_;
  int y_;
  int w_;
  int h_;

  // Color
  Color color_;

  // Internal signal for every segment
  std::vector<bool> segments_ = {0, 0, 0, 0, 0, 0, 0};

 public:

  Digit();
  Digit(SDL_Renderer *r);
  ~Digit();

  void initialize(SDL_Renderer *r);
  void setColor(const Color color);
  void setPosition(const int x, const int y);
  void setSize(const int w, const int h);

  void draw(std::vector<bool> seg);
  void draw(const char c);

};

#endif /* DIGIT_H */
