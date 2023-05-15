#ifndef NUMBER_H
#define NUMBER_H

// Parent clases
#include "Draw.h"

// Required classes
#include "Digit.h"
#include "Sign.h"
#include "Color.h"

// Standard Library
#include <vector>
#include <string>

#define N_SPACE_FACTOR (1)

class Number : public Draw {

 private:

  // Array of digits
  std::vector<Digit *> digits_;

  // Sign
  Sign * sign_;
  bool showSign_;

  // Value to print
  std::string value_;

  Digit *d_[5];

  Digit *a_ = NULL;

 public:

  /**
   * @brief Construct a new Number object
   * 
   * @param r [in] Pointer to the SDL Renderer
   * @param digits [in] Number of digits to display
   * @param x [in] X position
   * @param y [in] Y position
   * @param c [in] Color
   * @param glow [in] Glow enabled?
   */
  Number(SDL_Renderer * r, const int digits, const int x, const int y, const Color c, const bool glow);
  
  /**
   * @brief Set the Value object
   * 
   * @param s [in] String with the value to display
   */
  void setValue(const std::string s);

  /**
   * @brief Set the Position object
   * 
   * @param x [in] X position
   * @param y [in] Y position
   */
  void setPosition(const int x, const int y);

  // Base class virtual functions

  /**
   * @brief Behaviour when drawing the object
   * 
   */
  void drawOn();

  /**
   * @brief Behaviour when the drawing of the object is disabled
   * @note for the case of the number, the background digits will be
   * shown.
   */
  void drawOff();

  /**
   * @brief Enable the sign for the Number
   * 
   */
  void enableSign();

  /**
   * @brief Disable the sign for the Number
   * 
   */
  void disableSign();


};

#endif /* NUMBER_H */
