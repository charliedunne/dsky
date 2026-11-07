#ifndef VERBLISTVIEW_H
#define VERBLISTVIEW_H

#include <array>
#include <string>
#include <vector>

#include "LeftLcdView.h"

/**
 * @brief Left-LCD content view for list screens (VERB 0 menu and the like).
 *
 * Holds up to three pages of list lines plus a per-page header. Immutable
 * once published; a page flip publishes a copy with a different active page.
 */
class VerbListView : public LeftLcdView {
public:
  /**
   * @brief Pages of list lines (<=20 lines / 27 chars each, enforced by the
   *        HmiData::showVerbList() builder).
   */
  std::array<std::vector<std::string>, 3> pages{};

  /**
   * @brief Per-page header line; empty = use the default header for that page.
   */
  std::array<std::string, 3> headers{};

  /**
   * @brief Active page index (0..2).
   */
  int page = 0;

  /**
   * @brief Draw the active page through the frame's list canvas primitive.
   */
  void draw(FrameLeftLcd &f) const override;

  /**
   * @brief Short name for logs/debug.
   */
  const char *name() const override { return "VerbList"; }
};

#endif /* VERBLISTVIEW_H */
