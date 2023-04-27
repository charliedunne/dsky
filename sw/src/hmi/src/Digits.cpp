#include "Digits.h"
#include "config.h"
#include "Logger.h"
#include <string>

// Initialization of static members
bool Digits::ttfInitialized_ = false;
int Digits::instanceCounter_ = 0;

Digits::Digits() {

  // Initialization of the TTF (only once)
  // if (!ttfInitialized_) {

    if (TTF_Init() < 0) {

      LogError << "TTF_Init: " << SDL_GetError() << std::endl;
    }
    else {

      ttfInitialized_ = true;
    }
  // }

  if (ttfInitialized_) {

    font_ = TTF_OpenFont(ZERLINA_FONT, 72);
    if (font_ == NULL) {

      LogError << "TTF_OpenFont: " << SDL_GetError() << std::endl;
    }
  }

  // Default value
  value_ = "88";

  // Default status
  status_ = DIGITS_OFF;

  // Default color
  fgColor_ = Color(0, 255, 0, 255);
  bgColor_ = Color(0, 0, 0, 0);
  fgColorDisable_ = Color(40, 20, 20, 128);

  // Increase instance counter
  instanceCounter_++;


}


Digits::~Digits() {

  TTF_CloseFont(font_);

  // Decrease the instance counter
  instanceCounter_--;

  // The last instance to be destroyed also quit the SDL_ttf module
  // if (instanceCounter_ == 0) {

    TTF_Quit();
  // }
}

void Digits::setLongValue(int value) {

  std::string sign = "+";

  if (value < 0) {

    sign = '-';
  }

  value_ = sign + std::to_string(value);
}

void Digits::setShortValue(int value) {

  value_ = std::to_string(value);
}

void Digits::setValue(std::string value) {

  value_ = value;
}

void Digits::setPosition(int x, int y) {

  x_ = x;
  y_ = y;
}

void Digits::draw(SDL_Renderer *ren) {

  SDL_Surface *text = NULL;
  SDL_Texture *texture = NULL;

  // Generate the surface
  if (status_ == DIGITS_ON) {
    text = TTF_RenderText_Shaded(font_, value_.c_str(), fgColor_, bgColor_);

  }
  else {

    text = TTF_RenderText_Shaded(font_, value_.c_str(), fgColorDisable_, bgColor_);
  }

  // Generate the Texture from the surface
  texture = SDL_CreateTextureFromSurface(ren, text);

  // Draw the texture
  SDL_Rect dest = {x_, y_, text->w, text->h};
  SDL_RenderCopy(ren, texture, NULL, &dest);

  // Free resources
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(text);
}

void Digits::switchOn() {

  if (status_ == DIGITS_OFF) {

    status_ = DIGITS_ON;
  }
}

void Digits::switchOff() {

  if (status_ == DIGITS_ON) {

    status_ = DIGITS_OFF;
  }
}
