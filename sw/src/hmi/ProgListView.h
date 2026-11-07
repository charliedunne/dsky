#ifndef PROGLISTVIEW_H
#define PROGLISTVIEW_H

#include <string>
#include <vector>

#include "LeftLcdView.h"

/**
 * @brief Left-LCD content view for the running-programs list screens.
 *
 * Used by VERB 6 (LIST OF RUNNING PROGRAMS) and VERB 7 (KILL RUNNING
 * PROGRAMS). Holds an immutable snapshot of one pre-formatted row per running
 * program ("Pxx LABEL", or a single "NONE" row when idle) plus the index of
 * the currently selected row and the header string. Rows are built by the
 * caller from ListOfPrograms::describeRunning() so they already contain only
 * glyphs the Gorton bitmap font can draw; the canvas renders them and
 * highlights the selected row with a cursor block. Immutable once published.
 */
class ProgListView : public LeftLcdView {
public:
  /**
   * @brief One pre-formatted program row, e.g. "P05 MOONPHASE".
   */
  std::vector<std::string> lines;

  /**
   * @brief Index into lines of the selected program (-1 = none). The canvas
   *        draws a cursor block to the left of this row.
   */
  int selected = -1;

  /**
   * @brief Header line drawn above the separator (empty = "RUNNING PROGRAMS").
   */
  std::string header = "RUNNING PROGRAMS";

  /**
   * @brief Draw the program rows and selection cursor through the frame's
   *        shared value-list (rows) canvas primitive.
   */
  void draw(FrameLeftLcd &f) const override;

  /**
   * @brief Short name for logs/debug.
   */
  const char *name() const override { return "ProgList"; }
};

#endif /* PROGLISTVIEW_H */
