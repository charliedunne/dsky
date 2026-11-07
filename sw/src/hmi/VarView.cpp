#include "VarView.h"

#include "FrameLeftLcd.h"

void VarView::draw(FrameLeftLcd &f) const
{
  f.canvasRows(lines, selected, header);
}
