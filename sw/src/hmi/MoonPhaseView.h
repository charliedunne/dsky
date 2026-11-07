#ifndef MOONPHASEVIEW_H
#define MOONPHASEVIEW_H

#include "LeftLcdView.h"

/**
 * @brief Left-LCD content view for the MOONPHASE result.
 *
 * Holds the illumination percentage, the lunar phase and the date-correction
 * blink flag. Immutable once published; re-showing a corrected value publishes
 * a fresh view (or a copy with a different editBlink).
 */
class MoonPhaseView : public LeftLcdView {
public:
  /**
   * @brief Illumination percentage (0..100).
   */
  int pct = 0;

  /**
   * @brief Lunar phase in [0,1): 0=new, 0.5=full. Drives the disk graphic.
   */
  float phase = 0.0f;

  /**
   * @brief True while the date is being corrected after a result: a blinking
   *        X is drawn over the disk center (values are not valid yet).
   */
  bool editBlink = false;

  /**
   * @brief Draw the moon graphic, percentage and optional blink X through
   *        the frame's moon canvas primitive.
   */
  void draw(FrameLeftLcd &f) const override;

  /**
   * @brief Short name for logs/debug.
   */
  const char *name() const override { return "MoonPhase"; };
};

#endif /* MOONPHASEVIEW_H */
