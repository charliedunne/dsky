#ifndef VARVIEW_H
#define VARVIEW_H

#include <string>
#include <vector>

#include "LeftLcdView.h"

/**
 * @brief Left-LCD content view for the READ/WRITE VAR (VERB 8/9) screen.
 *
 * Holds an immutable snapshot of one pre-formatted row per variable plus the
 * index of the currently selected row and the header string. Rows are built by
 * the caller (ReadVar/WriteVar) so they already contain only glyphs the Gorton
 * bitmap font can draw; each row shows the variable's NOUN, label and current
 * value. The canvas renders them and highlights the selected row with a cursor
 * block. Immutable once published; each selection change publishes a fresh view.
 */
class VarView : public LeftLcdView {
public:
  /**
   * @brief One pre-formatted variable row, e.g. "2 KEYPAD LIGHT 1".
   */
  std::vector<std::string> lines;

  /**
   * @brief Index into lines of the selected variable (-1 = none). The canvas
   *        draws a cursor block to the left of this row.
   */
  int selected = -1;

  /**
   * @brief Header line drawn above the separator (empty = "VARIABLES").
   */
  std::string header = "VARIABLES";

  /**
   * @brief Draw the variable rows and selection cursor through the frame's
   *        shared value-list (rows) canvas primitive.
   */
  void draw(FrameLeftLcd &f) const override;

  /**
   * @brief Short name for logs/debug.
   */
  const char *name() const override { return "Variables"; }
};

#endif /* VARVIEW_H */
