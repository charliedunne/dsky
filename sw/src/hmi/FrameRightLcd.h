#ifndef FRAMERIGHTLCD_H
#define FRAMERIGHTLCD_H


#include "SDL2/SDL.h"
#include "Frame.h"
#include "Color.h"
//#include "Digits.h"
#include "Label.h"
#include "Digit.h"
#include "Number.h"

class FrameRightLcd : public Frame {

 private:

  Number * nProg_ = NULL;
  Number * nVerb_ = NULL;
  Number * nNoun_ = NULL;
  Number * nR1_ = NULL;
  Number * nR2_ = NULL;
  Number * nR3_ = NULL;

  Label * lProg_ = NULL;
  Label * lVerb_ = NULL;
  Label * lNoun_ = NULL;
  Label * lCompActy_ = NULL;
  Label * lR1_ = NULL;
  Label * lR2_ = NULL;
  Label * lR3_ = NULL;


  Color ghostColor_;
  Color liveColor_;
  Color dotsColor_;
  Color lineColor_;

  const bool debug_ = false;

 public:

  // Constructor
  FrameRightLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg); 

  // Destructor
  ~FrameRightLcd();

  // Local implementation of Render
  void render();

  // Configuration functions
  void setBgColor(const Color c);
  void setGhostColor(const Color c);
  void setLiveColor(const Color c);
  void setDotsColor(const Color c);
  void setlineColor(const Color c);


  // Setters for numeric values
  void setProg(const int value, DrawMode const mode = DRAW_ON);
  void setVerb(const int value, DrawMode const mode = DRAW_ON);
  void setNoun(const int value, DrawMode const mode = DRAW_ON);
  void setR1(const int value, DrawMode const mode = DRAW_ON);
  void setR2(const int value, DrawMode const mode = DRAW_ON);
  void setR3(const int value, DrawMode const mode = DRAW_ON);

  // Setters for Labels
  void setProgStatus(DrawMode);
  void setVerbStatus(DrawMode);
  void setNounStatus(DrawMode);
  void setCompActyStatus(DrawMode);
  void setR1Status(DrawMode);
  void setR2Status(DrawMode);
  void setR3Status(DrawMode);

private:

  // Drawing primitives
  void drawFixedElements();
  
  void compActy(Color c);
  void progDigits();



};

#endif /* FRAMERIGHTLCD_H */
