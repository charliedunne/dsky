#include "TempView.h"

#include "FrameLeftLcd.h"

void TempView::draw(FrameLeftLcd &f) const
{
  f.canvasTemp(lines, selected, header);
}
