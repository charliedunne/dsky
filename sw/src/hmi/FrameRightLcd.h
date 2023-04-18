#include "Frame.h"
#include "Digits.h"
#include "Label.h"

class FrameRightLcd : public Frame {

 private:
  
  // Frame elements
  Label lActComp_;
  Label lProg_;
  Label lVerb_;
  Label lNoun_;

  Digit dProg_;
  Digit dVerb_;
  Digit dNoun_;

  Digit dTop_;
  Digit dMid_;
  Digit dLow_;

 public:

  // Constructor
  FrameRightLcd(xPos, yPos, xSize, ySize)
    :Frame(xPos, yPos, xSize, ySize);

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
}
