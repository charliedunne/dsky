#ifndef FRAMERIGHTLCD_H
#define FRAMERIGHTLCD_H


#include "SDL2/SDL.h"
#include "Frame.h"
#include "Color.h"
//#include "Digits.h"
//#include "Label.h"
#include "Digit.h"
#include "Number.h"

class FrameRightLcd : public Frame {

 private:
  
  // Frame elements
  // Label lActComp_;
  // Label lProg_;
  // Label lVerb_;
  // Label lNoun_;

  // Digit dProg_;
  // Digit dVerb_;
  // Digit dNoun_;

  // Digit dTop_;
  // Digit dMid_;
  // Digit dLow_;

  Number * nProgBg_ = NULL;
  Number * nVerbBg_ = NULL;
  Number * nNounBg_ = NULL;
  Number * nR1Bg_ = NULL;
  Number * nR2Bg_ = NULL;
  Number * nR3Bg_ = NULL;

  Number * nProg_ = NULL;
  Number * nVerb_ = NULL;
  Number * nNoun_ = NULL;
  Number * nR1_ = NULL;
  Number * nR2_ = NULL;
  Number * nR3_ = NULL;

 public:

  // Constructor
  FrameRightLcd(SDL_Renderer *r, int xPos, int yPos, int xSize, int ySize, Color bg); 

  // Destructor
  ~FrameRightLcd();

  // Local implementation of Render
  void render();

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

  void setDigitProg(unsigned int value);
  void setDigitVerb(unsigned int value);
  void setDigitNoun(unsigned int value);
  void setDigitTop(unsigned int value);
  void setDigitMid(unsigned int value);
  void setDigitLow(unsigned int value);

private:

  // Drawing primitives
  void frameBoundaries(Color c);
  void compActy(Color c);
  void background();
  void progDigits();

};

#endif /* FRAMERIGHTLCD_H */
