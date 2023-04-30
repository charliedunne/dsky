#include "Digit.h"
#include "SDL2/SDL_image.h"
#include <Logger.h>

#define DIGIT_TXT_FILE "/home/cav/dsky/sw/resources/images/DigitsSpriteTemplate.png"
#define DIGIT_GLOW_TXT_FILE "/home/cav/dsky/sw/resources/images/DigitsSpriteTemplate_glow.png"
#define S_WIDTH 140
#define S_HEIGHT 160

#include <iostream>

Digit::Digit() {

  // Save renderer
  r_ = NULL;

  // Initialize internal private members
  x_ = 0;
  y_ = 0;
  w_ = S_WIDTH * 0.4;
  h_ = S_HEIGHT * 0.4;
  color_ = Color(0, 255, 0, 255);

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

void Digit::draw(std::vector<bool> seg) {

  // Create output rectange for drawing
  SDL_Rect dest = {x_, y_, w_, h_};

  // Create source from the Sprite
  SDL_Rect src = {0, 0, S_WIDTH, S_HEIGHT};

  segments_ = seg;

  for (int i = 0; i < seg.size(); ++i) {

    if (seg[i]) {

      // Set-up the src depending on the segment
      src.x = i* S_WIDTH;

      // Set the color
      SDL_SetTextureColorMod(digitTx_, 0, 255, 0);
      SDL_SetTextureColorMod(digitGlowTx_, 0, 255, 0);

      // Draw it
      SDL_RenderCopy(r_, digitGlowTx_, &src, &dest);
      SDL_RenderCopy(r_, digitTx_, &src, &dest);
    }
  }
}

