#ifndef COLOR_H
#define COLOR_H

#include "SDL2/SDL.h"

class Color {

 private:
  int red_ = 0;
  int green_ = 0;
  int blue_ = 0;
  int alpha_ = 0;

 public:

  // Constructor
  Color();
  Color(int r, int g, int b);
  Color(int r, int g, int b, int a);

  /** @brief Centralized color palette. All application colors are defined here
   * so the look of the HMI stays homogeneous and can be tuned from one place.
   * Values come from the color macros in config.h */
  /** @brief Live display green (primary readout color) */
  static Color live();
  /** @brief Ghost/unlit element color (dark background tint) */
  static Color ghost();
  /** @brief Alarm yellow (warning states on the left LCD) */
  static Color alarmYellow();
  /** @brief Default label green (dim green used for PROG/VERB/NOUN labels) */
  static Color labelGreen();
  /** @brief Faint dot grid overlay color */
  static Color dots();
  /** @brief Faint separator line color */
  static Color line();
  /** @brief Pure black */
  static Color black();
  /** @brief Pure white (debug strings) */
  static Color white();
  /** @brief Error red (error text/highlights) */
  static Color errorRed();
  /** @brief Fully transparent */
  static Color transparent();
  /** @brief Dimmed/inactive element color */
  static Color disabled();
  /** @brief Bright green (fully saturated; X-cross, list text, moon lit part/edge) */
  static Color brightGreen();
  /** @brief Divider green (dim green separator line in the verb list) */
  static Color dividerGreen();
  /** @brief Moon dim green (unlit part of the moon disk) */
  static Color moonDimGreen();
  /** @brief Debug yellow (debug frame border; opaque, unlike alarmYellow) */
  static Color debugYellow();

  // Getters
  int r() const;
  int g() const;
  int b() const;
  int a() const;

  // Setters
  void set(int r, int g, int b);
  void set(int r, int g, int b, int a);

  // Cast overload
  operator int() const;
  operator SDL_Color() const;

};

#endif /* COLOR_H */
