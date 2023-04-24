#include "Color.h"

Color::Color() {

  red_ = 0;
  green_ = 0;
  blue_ = 0;
  alpha_ = 0;
}


Color::Color(int r, int g, int b) {

  red_ = r;
  green_ = g;
  blue_ = b;
  alpha_ = 255;
}

Color::Color(int r, int g, int b, int a) {

  red_ = r;
  green_ = g;
  blue_ = b;
  alpha_ = a;
}

int Color::r() {

  return red_;
}

int Color::g() {

  return green_;
}

int Color::b() {

  return blue_;
}

int Color::a() {

  return alpha_;
}

void Color::set(int r, int g, int b) {

  red_ = r;
  green_ = g;
  blue_ = b;
}

void Color::set(int r, int g, int b, int a) {

  red_ = r;
  green_ = g;
  blue_ = b;
  alpha_ = a;
}


Color::operator int() const {

  int output = blue_ | green_ << 8 | red_ << 16 | alpha_ << 24;

  return output;
}

Color::operator SDL_Color() const {

  SDL_Color output = {(Uint8)red_, (Uint8)green_, (Uint8)blue_, (Uint8)alpha_};

  return output;
}
