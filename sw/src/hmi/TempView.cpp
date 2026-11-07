#include "TempView.h"

#include "FrameLeftLcd.h"

void TempView::draw(FrameLeftLcd &f) const
{
  f.canvasRows(lines, selected, header);
}
