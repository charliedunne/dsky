#include "MoonPhaseView.h"

#include "FrameLeftLcd.h"

void MoonPhaseView::draw(FrameLeftLcd &f) const
{
  f.canvasMoon(phase, pct, editBlink);
}
