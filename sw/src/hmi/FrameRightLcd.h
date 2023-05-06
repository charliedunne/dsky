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

  Color ghostColor_;
  Color liveColor_;
  Color dotsColor_;
  Color lineColor_;

  const bool debug_ = true;

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


  // Operation functions
  void enableLabelActComp();
  void disableLabelActComp();
  void enableLabelProg();
  void disableLabelProg();
  void enableLabelVerb();
  void disableLabelVerb();
  void enableLabelNoun();
  void disableLabelNoun();
  void enableDigitProg();
  void disableDigitProg();
  void enableDigitVerb();
  void disableDigitVerb();
  void enableDigitNoun();
  void disableDigitNoun();
  void enableDigitTop();
  void disableDigitTop();
  void enableDigitMid();
  void disableDigitMid();
  void enableDigitLow();
  void disableDigitLow();

  void setProg(int);
  void setVerb(int);
  void setNoun(int);
  void setR1(int);
  void setR2(int);
  void setR3(int);

private:

  // Drawing primitives
  void drawFixedElements();
  
  void compActy(Color c);
  void progDigits();



};

#endif /* FRAMERIGHTLCD_H */
