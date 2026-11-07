#ifndef LEFTLCDVIEW_H
#define LEFTLCDVIEW_H

class FrameLeftLcd;   // forward decl — no circular include

/**
 * @brief A single left-LCD content view. Immutable once published.
 */
class LeftLcdView {
public:
  virtual ~LeftLcdView() = default;

  /**
   * @brief Render this view's content using the frame as its canvas.
   */
  virtual void draw(FrameLeftLcd &frame) const = 0;

  /**
   * @brief Short name for logs/debug.
   */
  virtual const char *name() const { return "view"; }
};

#endif /* LEFTLCDVIEW_H */
