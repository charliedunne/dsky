#ifndef FRAMELEFTLCD_H
#define FRAMELEFTLCD_H

#include <string>
#include <vector>

#include "SDL2/SDL.h"
#include "Frame.h"
#include "Color.h"
#include "Label.h"
#include "Font.h"
#include "Number.h"

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
  /**
   * @brief Update the shared SHUTDOWN/REBOOT alarm label.
   *
   * Both verbs share one grid cell and only one can be active at a time
   * (ACTION_SINGLE). The first non-OFF mode selects the text and blink state.
   */
  void setShutdownReboot(DrawMode shMode, DrawMode rbMode);

  /**
   * @brief Update the VERB 0 menu content from the left LCD data.
   */
  void setVerbList(const LeftLcdData &d);

  /**
   * @brief Update the MOONPHASE illumination display.
   *
   * DRAW_ON shows the given percentage (0..100) with a "%" label;
   * any other mode hides it.
   */
  void setMoon(DrawMode mode, int pct);


private:
  // VERB 0 menu rendering
  Font *verbListFont_ = nullptr;
  std::vector<std::string> verbListLines_;   // lines of the active page
  DrawMode verbListMode_ = DRAW_OFF;
  int verbListPage_ = 0;                     // active page index (0 or 1)
  std::string verbListHeader_;               // header line of the active page (empty = default)

  // MOONPHASE illumination display
  Number *nMoon_ = nullptr;                  // large centered percentage digits (3 digits)
  Font *moonPctFont_ = nullptr;              // "%" label font
  DrawMode moonMode_ = DRAW_OFF;
  int moonPct_ = 0;

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

  /**
   * @brief Internal: draw the MOONPHASE percentage digits and "%" label
   */
  void drawMoon();
};

#endif /* FRAMELEFTLCD_H */
