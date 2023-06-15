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
  AlarmLabel *opErr = nullptr;
  AlarmLabel *spare2 = nullptr;

  SDL_Texture *gridTxt_ = nullptr;

public:
  // Constructor
  FrameLeftLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg);

  // Destructor
  ~FrameLeftLcd();

  // Local implementation of Render
  void render();

  // Setters for alarms and status
  void setUplink(DrawMode);
  void setTemp(DrawMode);
  void setKeyErr(DrawMode);
  void setOpErr(DrawMode);


private:
  /**
   * @brief Draw the background lines (grid)
   *
   */
  void drawLines();

  /**
   * @brief draw the alarm labels
   * 
   */
  void drawLabels();
};

#endif /* FRAMELEFTLCD_H */
