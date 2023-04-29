#include "Digit.h"

#define BMP_FILE "/home/cav/dsky/sw/resources/images/DigitsSpriteTemplate.bmp"

#define S_WIDTH 140
#define S_HEIGHT 160

#define N_SEGMENTS 7

Digit::Digit(SDL_Renderer r) {

  // Save renderer
  r_ = r;

  // Initialize internal private members
  x_ = 0;
  y_ = 0;
  w_ = S_WIDTH;
  h_ = S_HEIGHT;
  color_ = Color(0, 255, 0, 255);

  // Create a surface from the BMP file
  SDL_Surface * surface = SDL_LoadBMP(BMP_FILE);

  // Create the Texture
  texture_ = SDL_CreateTextureFromSurface(r_, surface);

  // Release the surface resources
  SDL_FreeSurface(surface);

  // Reset segments
  for (int i = 0; i < N_SEGMENTS; ++i) {

    segments_[i] = false;
  }

}

Digit::~Digit() {

  // Destroy the texture resources
  SDL_DestroyTexture(texture_);
}

void Digit::setColor(const Color color) {

  color_ = color;
}

void Digit::setPosition(const int x, const int y) {

  x_ = x;
  y_ = y;
}

void Digit::setSize(const int w, const int h) {

  w_ = w;
  h_ = h;
}

void Digits::draw(const bool segments[N_SEGMENTS]) const {

  // Create output rectange for drawing
  SDL_Rect dest = {x_, y_, w_, h_};

  // Create source from the Sprite
  SDL_Rect src = {0, 0, S_WIDTH, S_HEIGHT};

  for (int i = 0; i < N_SEGMENTS; ++i) {

    // Check if the segment must be displayed
    if (segments_[i]) {

      // Set-up the src depending on the segment
      src.x = i*N_WIDTH;

      // Set the color
      SDL_SetTextureColorMod(texture_, 0, 255, 0);

      // Draw it
      SDL_RenderCopy(r_, texture_, &src, &dest);
    }
  }
}
