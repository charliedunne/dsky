#include "VerbListView.h"

#include "FrameLeftLcd.h"

void VerbListView::draw(FrameLeftLcd &f) const
{
  // Resolve the header: use the per-page override when set, otherwise fall
  // back to the defaults (page 0 = verb list, page 1 = variable list,
  // page 2 = program list). The canvas primitive receives a concrete header.
  std::string header = headers[page];
  if (header.empty()) {
    switch (page) {
      case 0:  header = "VERB LIST"; break;
      case 1:  header = "VARIABLE LIST"; break;
      default: header = "PROGRAM LIST"; break;
    }
  }

  f.canvasList(pages[page], header);
}
