#include "Color.h"
#include "config.h"

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

int Color::r() const {

  return red_;
}

int Color::g() const {

  return green_;
}

int Color::b() const {

  return blue_;
}

int Color::a() const {

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

  int output = red_ | green_ << 8 | blue_ << 16 | alpha_ << 24;

  return output;
}

Color::operator SDL_Color() const {

  SDL_Color output = {(Uint8)red_, (Uint8)green_, (Uint8)blue_, (Uint8)alpha_};

  return output;
}


/** @brief Live display green (primary readout color) */
Color Color::live() {

  return Color(LIVE_COLOR_R, LIVE_COLOR_G, LIVE_COLOR_B, LIVE_COLOR_A);
}

/** @brief Ghost/unlit element color (dark background tint) */
Color Color::ghost() {

  return Color(GHOST_COLOR_R, GHOST_COLOR_G, GHOST_COLOR_B, GHOST_COLOR_A);
}

/** @brief Alarm yellow (warning states on the left LCD) */
Color Color::alarmYellow() {

  return Color(ALARM_YELLOW_COLOR_R, ALARM_YELLOW_COLOR_G, ALARM_YELLOW_COLOR_B, ALARM_YELLOW_COLOR_A);
}

/** @brief Default label green (dim green used for PROG/VERB/NOUN labels) */
Color Color::labelGreen() {

  return Color(LABEL_GREEN_COLOR_R, LABEL_GREEN_COLOR_G, LABEL_GREEN_COLOR_B, LABEL_GREEN_COLOR_A);
}

/** @brief Faint dot grid overlay color */
Color Color::dots() {

  return Color(DOTS_COLOR_R, DOTS_COLOR_G, DOTS_COLOR_B, DOTS_COLOR_A);
}

/** @brief Faint separator line color */
Color Color::line() {

  return Color(LINE_COLOR_R, LINE_COLOR_G, LINE_COLOR_B, LINE_COLOR_A);
}

/** @brief Pure black */
Color Color::black() {

  return Color(0, 0, 0);
}

/** @brief Pure white (debug strings) */
Color Color::white() {

  return Color(255, 255, 255);
}

/** @brief Error red (error text/highlights) */
Color Color::errorRed() {

  return Color(ERROR_RED_COLOR_R, ERROR_RED_COLOR_G, ERROR_RED_COLOR_B);
}

/** @brief Fully transparent */
Color Color::transparent() {

  return Color(0, 0, 0, 0);
}

/** @brief Dimmed/inactive element color */
Color Color::disabled() {

  return Color(DISABLED_COLOR_R, DISABLED_COLOR_G, DISABLED_COLOR_B, DISABLED_COLOR_A);
}
