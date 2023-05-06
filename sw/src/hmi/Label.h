#ifndef LABEL_H
#define LABEL_H

// Graphics Libraries
#include "SDL2/SDL.h"

// External configuration
#include "config.h"

// Parent clases
#include "Draw.h"

// Required Classes
#include "Color.h"


#define L_WIDTH 150
#define L_HEIGHT 50
#define L_FACTOR (0.5)

typedef enum
{
  LABEL_PROG,
  LABEL_VERB,
  LABEL_NOUN,
  LABEL_COMPACTY,
  LABEL_LINE
} LabelType;

class Label : public Draw
{

private:
  // Renderer
  SDL_Renderer *r_ = NULL;

  // Texture with the gliphs
  SDL_Texture *labelTx_ = NULL;

  // Size
  int w_;
  int h_;

  // Foreground color
  Color fgColor_;

  // Background color
  Color bgColor_;

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
  void setColor(const Color fgColor,
                const Color bgColor =
                    Color(GHOST_COLOR_R,
                          GHOST_COLOR_G,
                          GHOST_COLOR_B,
                          GHOST_COLOR_A));
  ;
  void setBgColor(const Color c);
  void setPosition(const int x, const int y);

   // Set the status 
  void setStatus(const bool);

  // Base class virtual functions
  void drawOn();
  void drawOff();
};

#endif /* LABEL_H */
