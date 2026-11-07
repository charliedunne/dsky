#include "ProgListView.h"

#include "FrameLeftLcd.h"

void ProgListView::draw(FrameLeftLcd &f) const
{
  f.canvasRows(lines, selected, header);
}
