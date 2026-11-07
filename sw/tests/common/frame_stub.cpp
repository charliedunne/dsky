// Link-time stubs for left-LCD frame canvas primitives.
//
// View classes (e.g. VerbListView) reference FrameLeftLcd::canvasList from
// their draw() implementation. The data/logic test suites compile those view
// translation units (their vtables live there) but never render a frame, so
// the real FrameLeftLcd.cpp - which drags in SDL2 - is not linked. This stub
// satisfies the linker; it must stay behaviorally inert since draw() is never
// invoked from these suites.

#include "FrameLeftLcd.h"

void FrameLeftLcd::canvasList(const std::vector<std::string> &, const std::string &) const {}

void FrameLeftLcd::canvasMoon(float, int, bool) const {}

void FrameLeftLcd::canvasRows(const std::vector<std::string> &, int, const std::string &) const {}
