#ifndef FRAMELEFTLCD_H
#define FRAMELEFTLCD_H

#include <string>
#include <vector>

#include "SDL2/SDL.h"
#include "Frame.h"
#include "Color.h"
#include "Label.h"
#include "Font.h"

// HMI
#include "AlarmLabel.h"
#include "HmiData.h"

class FrameLeftLcd : public Frame
{

private:
  const bool debug_ = false;

  int lineThickness_ = 15;

  AlarmLabel *uplinkAct = nullptr;
  AlarmLabel *temp = nullptr;
  AlarmLabel *keyErr = nullptr;
  AlarmLabel *spare1 = nullptr;
  AlarmLabel *opErr = nullptr;
  AlarmLabel *shutdown = nullptr;

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
  void setShutdown(DrawMode);
  void setReboot(DrawMode);

  /**
   * @brief Update the VERB 0 menu content from the left LCD data.
   */
  void setVerbList(const LeftLcdData &d);


private:
  // VERB 0 menu rendering
  Font *verbListFont_ = nullptr;
  std::vector<std::string> verbListLines_;   // lines of the active page
  DrawMode verbListMode_ = DRAW_OFF;
  int verbListPage_ = 0;                     // active page index (0 or 1)

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

  /**
   * @brief Internal: draw the VERB 0 menu lines over the grid background
   */
  void drawVerbList();
};

#endif /* FRAMELEFTLCD_H */
