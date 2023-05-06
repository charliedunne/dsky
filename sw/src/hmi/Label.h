#ifndef LABEL_H
#define LABEL_H

#include "Color.h"
#include "SDL2/SDL.h"

#include "Color.h"

#define L_WIDTH 150
#define L_HEIGHT 50
#define L_FACTOR (0.5)

typedef enum
{
  LABEL_PROG,
  LABEL_VERB,
  LABEL_NOUN,
  LABEL_COMPACTY
} LabelType;

class Label
{

private:
  // Renderer
  SDL_Renderer *r_ = NULL;

  // Texture with the gliphs
  SDL_Texture *labelTx_ = NULL;

  // Position and size
  int x_;
  int y_;
  int w_;
  int h_;

  // Foreground color
  Color fgColor_;

  // Background color
  Color bgColor_;

  // Status
  bool enabled_ = false;

  // Label type
  LabelType lType_;

public:
  /**
   * @brief Construct a new Label object
   *
   * @param r [in] Pointer to the Renderer
   */
  Label(SDL_Renderer *r,
        const int x, const int y, const int w, const int h,
        const LabelType t);

  // Destructor
  ~Label();

  // Setters and Getters
  void setFgColor(const Color c);
  void setBgColor(const Color c);
  void setPosition(const int x, const int y);

  void draw();

  // Enable Label
  void SwitchOn();

  // Disable Label
  void SwitchOff();

  // Blink Label
  void Blink(unsigned int freq);
};

#endif /* LABEL_H */
