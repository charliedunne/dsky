#include "Digit.h"
#include "config.h"
#include "SDL2/SDL_image.h"
#include <Logger.h>

#include <iostream>

std::vector<bool> getSegmentsFromChar(const char c) {

  std::vector<bool> segments = {0, 0, 0, 0, 0, 0, 0};

  switch (c) {

  case '0':
    segments[0] = 1;
    segments[1] = 1;
    segments[2] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[5] = 1;
    break;

  case '1':
    segments[1] = 1;
    segments[2] = 1;
    break;

  case '2':
    segments[0] = 1;
    segments[1] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[6] = 1;
    break;

  case '3':
    segments[0] = 1;
    segments[1] = 1;
    segments[2] = 1;
    segments[3] = 1;
    segments[6] = 1;
    break;

  case '4':
    segments[1] = 1;
    segments[2] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;

  case '5':
    segments[0] = 1;
    segments[2] = 1;
    segments[3] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;

  case '6':
    segments[0] = 1;
    segments[2] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;

  case '7':
    segments[0] = 1;
    segments[1] = 1;
    segments[2] = 1;
    break;

  case '8':
    segments[0] = 1;
    segments[1] = 1;
    segments[2] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;

  case '9':
    segments[0] = 1;
    segments[1] = 1;
    segments[2] = 1;
    segments[3] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;

  case 'a':
  case 'A':
    segments[0] = 1;
    segments[1] = 1;
    segments[2] = 1;
    segments[4] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;

  case 'b':
  case 'B':
    segments[2] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;

  case 'c':
  case 'C':
    segments[0] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[5] = 1;
    break;

  case 'd':
  case 'D':
    segments[1] = 1;
    segments[2] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[6] = 1;
    break;

  case 'e':
  case 'E':
    segments[0] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;

  case 'f':
  case 'F':
    segments[0] = 1;
    segments[4] = 1;
    segments[5] = 1;
    segments[6] = 1;
    break;


  default:
    segments[2] = 1;
    segments[3] = 1;
    segments[4] = 1;
    segments[6] = 1;
    break;
  }

  return segments;
}

Digit::Digit() {

  // Save renderer
  r_ = NULL;

  // Initialize internal private members
  x_ = 0;
  y_ = 0;
  w_ = S_WIDTH/2 * S_FACTOR;
  h_ = S_HEIGHT * S_FACTOR;
  color_ = Color(0, 255, 0, 255);
  glow_ = false;

}

Digit::Digit(SDL_Renderer *r) : Digit() {

  Digit::initialize(r);
}


void Digit::initialize(SDL_Renderer *r) {
  // Control flag
  bool ctrlFlag = true;

  // Save renderer
  r_ = r;

  // Create a surface from the BMP file
  SDL_Surface * surface = IMG_Load(DIGIT_TXT_FILE);
  if (surface == NULL) {

    LogError << "Fail loading the Segment texture (" << DIGIT_TXT_FILE << "): " << SDL_GetError() << std::endl;
    ctrlFlag = false;

  }

  SDL_Surface * surfaceGlow = IMG_Load(DIGIT_GLOW_TXT_FILE);
  if (surface == NULL) {

    LogError << "Fail loading the Segment texture (" << DIGIT_GLOW_TXT_FILE << "): " << SDL_GetError() << std::endl;
    ctrlFlag = false;
  }


  //  SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
  //  surface->format->Amask = 0xFF000000;
  //    surface->format->Ashift = 24;

  // Create the Texture
  if (ctrlFlag) {


    digitTx_ = SDL_CreateTextureFromSurface(r_, surface);
    if (digitTx_ == NULL) {

      LogError << "Fail in the Digit Texture creation: " << SDL_GetError() << std::endl;
    }

    digitGlowTx_ = SDL_CreateTextureFromSurface(r_, surfaceGlow);
    if (digitTx_ == NULL) {

      LogError << "Fail in the Glow Texture creation: " << SDL_GetError() << std::endl;
    }
  }

  // Release the surface resources
  SDL_FreeSurface(surface);
  SDL_FreeSurface(surfaceGlow);

}

Digit::~Digit() {

  // Destroy the texture resources
  SDL_DestroyTexture(digitTx_);
  SDL_DestroyTexture(digitGlowTx_);
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

void Digit::setGlow(const bool glow) {
  glow_ = glow;
} 

void Digit::draw(std::vector<bool> seg) {

  // Create output rectange for drawing
  SDL_Rect dest = {x_, y_, w_, h_};

  // Create source from the Sprite
  SDL_Rect src = {0, 0, S_WIDTH/2, S_HEIGHT};

  segments_ = seg;

  for (int i = 0; i < seg.size(); ++i) {

    if (seg[i]) {

      // Set-up the src depending on the segment
      src.x = i* S_WIDTH;

      // Set the color
      SDL_SetTextureColorMod(digitTx_, color_.r(), color_.g(), color_.b());
      SDL_SetTextureAlphaMod(digitTx_, color_.a());
      SDL_SetTextureColorMod(digitGlowTx_, color_.r(), color_.g(), color_.b());
      SDL_SetTextureAlphaMod(digitGlowTx_, color_.a());

      // Draw it
      if (glow_) { SDL_RenderCopy(r_, digitGlowTx_, &src, &dest); }
      SDL_RenderCopy(r_, digitTx_, &src, &dest);
    }
  }
}

void Digit::draw(const char c) {

  Digit::draw(getSegmentsFromChar(c));
}
