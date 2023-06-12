#ifndef FRAMELEFTLCD_H
#define FRAMELEFTLCD_H

#include "SDL2/SDL.h"
#include "Frame.h"
#include "Color.h"
#include "Label.h"

// HMI
#include "AlarmLabel.h"

class FrameLeftLcd : public Frame
{

private:
  const bool debug_ = true;

  int lineThickness_ = 15;

    AlarmLabel *uplinkAct = nullptr;
    AlarmLabel *temp = nullptr;
    AlarmLabel *keyErr = nullptr;
    AlarmLabel *spare1 = nullptr;
    AlarmLabel *opError = nullptr;
    AlarmLabel *spare2 = nullptr;
public:
  // Constructor
  FrameLeftLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg);

  // Destructor
  ~FrameLeftLcd();

  // Local implementation of Render
  void render();

private:

  /**
   * @brief Draw the background lines
   * 
   * @param c[in] Color of the linnnes
   * @param thickness[in] Thickness of the lines
  */
  void drawLines(Color c, int thickness);

  void drawLabels();

};

#endif /* FRAMELEFTLCD_H */
