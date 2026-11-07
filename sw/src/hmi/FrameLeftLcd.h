#ifndef FRAMELEFTLCD_H
#define FRAMELEFTLCD_H

#include <memory>
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
#include "LeftLcdView.h"

class FrameLeftLcd : public Frame
{

private:
  const bool debug_ = false;

  int lineThickness_ = 15;

  AlarmLabel *uplinkAct = nullptr;
  AlarmLabel *temp = nullptr;
  AlarmLabel *keyErr = nullptr;
  AlarmLabel *door = nullptr;
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

  /**
   * @brief Set the DOOR alarm label draw mode (remote UDP door signal).
   */
  void setDoor(DrawMode);

  void setOpErr(DrawMode);
  /**
   * @brief Update the shared SHUTDOWN/REBOOT alarm label.
   *
   * Both verbs share one grid cell and only one can be active at a time
   * (ACTION_SINGLE). The first non-OFF mode selects the text and blink state.
   */
  void setShutdownReboot(DrawMode shMode, DrawMode rbMode);

  /**
   * @brief Set the active left-LCD content view (nullptr = none).
   *        The frame is a pure canvas: it stores the immutable snapshot and
   *        delegates rendering to the view itself.
   */
  void setView(std::shared_ptr<const LeftLcdView> v);

  /**
   * @brief Canvas primitive: draw a list page (header + separator + up to
   *        19 lines, "..." when truncated) over the grid background. Used by
   *        list views via VerbListView::draw().
   */
  void canvasList(const std::vector<std::string> &lines, const std::string &header) const;

  /**
   * @brief Canvas primitive: draw the MOONPHASE result — "MOON PHASE" title,
   *        moon disk at the given phase, percentage digits with a "%" glyph
   *        below, and (when editBlink is on) a blinking X over the disk
   *        center indicating the values are not valid yet. Used by moon
   *        views via MoonPhaseView::draw().
   * @param phase Moon phase in [0,1): 0=new, 0.25=waxing quarter,
   *              0.5=full, 0.75=waning quarter.
   * @param pct Illumination percentage, 0..100.
   * @param editBlink true to overlay the blinking date-correction X.
   */
  void canvasMoon(float phase, int pct, bool editBlink) const;

  /**
   * @brief Canvas primitive: draw a value-list screen — header + separator +
   *        one pre-formatted row per entry, with a cursor block to the left of
   *        the selected row. Shared by any view whose content is a list of
   *        font-safe rows (e.g. TEMP MONITOR via TempView::draw() and VARIABLES
   *        via VarView::draw()).
   * @param lines Pre-formatted rows (font-safe glyphs only).
   * @param selected Index into lines of the selected row (-1 = none).
   * @param header Header line drawn above the separator (views set a default).
   */
  void canvasRows(const std::vector<std::string> &lines, int selected, const std::string &header) const;


private:
  // List canvas: font shared by list views and the MOONPHASE title
  Font *verbListFont_ = nullptr;

  /**
   * @brief Active left-LCD content view (nullptr = none). Same render thread
   *        as update(), so a plain member is safe.
   */
  std::shared_ptr<const LeftLcdView> view_;

  // MOONPHASE canvas resource: centered percentage digits (3 digits). Shared
  // by moon views via canvasMoon(); the frame keeps no moon state of its own.
  Number *nMoon_ = nullptr;

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
