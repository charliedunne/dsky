#ifndef TEMPVIEW_H
#define TEMPVIEW_H

#include <string>
#include <vector>

#include "LeftLcdView.h"

/**
 * @brief Left-LCD content view for the TEMP MONITOR (VERB 10) screen.
 *
 * Holds an immutable snapshot of one pre-formatted row per sensor plus the
 * index of the currently selected row and the header string. Rows are built by
 * the caller (TempMonitor) so they already contain only glyphs the Gorton
 * bitmap font can draw; the canvas renders them and highlights the selected
 * row with a cursor block. Immutable once published; each tick publishes a
 * fresh view with the latest temperatures.
 */
class TempView : public LeftLcdView {
public:
  /**
   * @brief One pre-formatted sensor row, e.g. "1 KITCHEN 23*5C".
   */
  std::vector<std::string> lines;

  /**
   * @brief Index into lines of the selected sensor (-1 = none). The canvas
   *        draws a cursor block to the left of this row.
   */
  int selected = -1;

  /**
   * @brief Header line drawn above the separator (empty = "TEMPERATURES").
   */
  std::string header = "TEMPERATURES";

  /**
   * @brief Draw the temperature rows and selection cursor through the frame's
   *        temp canvas primitive.
   */
  void draw(FrameLeftLcd &f) const override;

  /**
   * @brief Short name for logs/debug.
   */
  const char *name() const override { return "TempMonitor"; }
};

#endif /* TEMPVIEW_H */
