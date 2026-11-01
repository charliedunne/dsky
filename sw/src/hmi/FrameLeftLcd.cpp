#include "FrameLeftLcd.h"

// STL
#include <iostream>
#include <stdexcept>

// Configuration
#include "config.h"

// Logging
#include <Logger.h>

// GFX/SDL
#include "SDL2/SDL_image.h"
#include "GFX/SDL2_gfxPrimitives.h"

FrameLeftLcd::FrameLeftLcd(SDL_Renderer *r, int x, int y, int w, int h, Color bg)
    : Frame(r, x, y, w, h, bg)
{
    int extGridSize = 5;

    // Create the Alarms Label objects

    uplinkAct = new AlarmLabel(r_, extGridSize, extGridSize+2,
                               w_ / 2, (h_ / 2) / 3, std::string("UPLINK"));

    temp = new AlarmLabel(r_, w_ / 2 + 1, extGridSize+2,
                          w_ / 2, (h_ / 2) / 3, std::string("TEMP"));

    keyErr = new AlarmLabel(r_, extGridSize, (h_ / 2) / 3 + 1,
                            w_ / 2, (h_ / 2) / 3, std::string("KEY REL"));

    spare1 = new AlarmLabel(r_, w_ / 2 + 1, (h_ / 2) / 3 + 1,
                            w_ / 2, (h_ / 2) / 3, std::string("NONE"));

    opErr = new AlarmLabel(r_, extGridSize, 2 * (h_ / 2) / 3 + 2,
                             w_ / 2, (h_ / 2) / 3, std::string("OP ERR"));

    shutdown = new AlarmLabel(r_, w_ / 2 + 1, 2 * (h_ / 2) / 3 + 2,
                            w_ / 2, (h_ / 2) / 3, std::string("SHTDWN"));

    // Center the text ink on the measured grid-cell band centers
    // (window-absolute). The yellow box geometry is left unchanged; only
    // the text position moves.
    uplinkAct->setTargetCenter(91, 49);
    temp->setTargetCenter(248, 49);
    keyErr->setTargetCenter(91, 123);
    spare1->setTargetCenter(248, 123);
    opErr->setTargetCenter(91, 202);
    shutdown->setTargetCenter(248, 202);

    // Set the Alarm colors
    shutdown->setFgColor(Color(ALARM_YELLOW_COLOR_R, ALARM_YELLOW_COLOR_G, ALARM_YELLOW_COLOR_B, ALARM_YELLOW_COLOR_A));

    // VERB 0 menu font (Gorton bitmap, scaled down at draw time)
    verbListFont_ = new Font(r_, GORTON_FONT_BITMAP, GORTON_FONT_FNT);

    // Create the Grid object
    SDL_Surface *surface = IMG_Load(GRID_FULL_TXT_FILE);
    if (surface == NULL)
    {
        LogError << "Fail loading the Segment texture (" << GRID_FULL_TXT_FILE << "): " << SDL_GetError() << std::endl;
        throw std::domain_error("Fail opening grid image");
    }

    gridTxt_ = SDL_CreateTextureFromSurface(r_, surface);
    if (gridTxt_ == NULL)
    {
        LogError << "Fail in the creation of Grid texture" << std::endl;
        throw std::domain_error("Fail creating grid image");
    }
}

FrameLeftLcd::~FrameLeftLcd()
{
    delete uplinkAct;
    delete temp;
    delete keyErr;
    delete spare1;
    delete opErr;
    delete shutdown;
    delete verbListFont_;
}

void FrameLeftLcd::render()
{
    // Background Color
    boxColor(r_, x_, y_, x_ + w_, y_ + h_, bg_);

    // Frame perimeter
    if (debug_)
    {
        roundedRectangleColor(r_, x_, y_,
                              x_ + w_ - 1, y_ + h_ - 1,
                              M_PI * 8, Color(255, 255, 0));
    }

    // Draw alarm labels only when the VERB 0 menu is not shown
    if (verbListMode_ != DRAW_ON) {
        drawLabels();
    }

    // Draw the separation lines (grid background), hidden while the VERB 0 menu is shown
    if (verbListMode_ != DRAW_ON) {
        drawLines();
    }

    // Draw the VERB 0 menu on top of the grid, if active
    if (verbListMode_ == DRAW_ON) {
        drawVerbList();
    }
}

void FrameLeftLcd::drawLines()
{
    // Create output rectange for drawing
    SDL_Rect dest = {x_, y_, w_, h_};

    // Render the grid image
    SDL_RenderCopy(r_, gridTxt_, NULL, &dest);
}

void FrameLeftLcd::drawLabels()
{
    uplinkAct->draw();
    temp->draw();
    keyErr->draw();
    spare1->draw();
    opErr->draw();
    shutdown->draw();
}

  void FrameLeftLcd::setUplink(DrawMode mode)
  {
    uplinkAct->setMode(mode);
  }

  void FrameLeftLcd::setTemp(DrawMode mode)
  {
    temp->setMode(mode);
  }
  
  void FrameLeftLcd::setKeyErr(DrawMode mode)
  {
    keyErr->setMode(mode);
  }
  
  void FrameLeftLcd::setOpErr(DrawMode mode)
  {
    opErr->setMode(mode);
  }

  void FrameLeftLcd::setShutdownReboot(DrawMode shMode, DrawMode rbMode)
  {
    // Single deterministic setter for the shared label: only one verb can be
    // active at a time (ACTION_SINGLE), so the first non-OFF mode selects both
    // the text and the blink state. Calling two independent setters here used
    // to let the OFF side clobber the active side every frame.
    if (shMode != DRAW_OFF) {
      shutdown->setText("SHTDWN");
      shutdown->setMode(shMode);
    } else if (rbMode != DRAW_OFF) {
      shutdown->setText("REBOOT");
      shutdown->setMode(rbMode);
    } else {
      shutdown->setMode(DRAW_OFF);
    }
  }

void FrameLeftLcd::setVerbList(const LeftLcdData &d)
{
    verbListMode_ = d.lVerbList;
    verbListPage_ = d.lVerbListPage;

    // Copy only the lines of the active page.
    verbListLines_.clear();
    const int count = d.lVerbListCount[verbListPage_];
    for (int i = 0; i < count && i < 20; ++i) {
        verbListLines_.push_back(std::string(d.lVerbListLines[verbListPage_][i]));
    }

    // Copy the header of the active page (empty string = default header).
    verbListHeader_ = std::string(d.lVerbListHeader[verbListPage_]);
}

void FrameLeftLcd::drawVerbList()
{
    if (verbListMode_ != DRAW_ON) return;

    // Draw over the grid background, stacked from the top.
    // scale ~0.5 fits the ~32px Gorton bitmap into the 335px-wide panel.
    const float scale = 0.5f;
    const int lineHeight = (int)(32 * scale) + 6;   // glyph height + gap (tune later)
    const Color green(0, 255, 0, 255);

    // Header line: use the data-provided header when set, otherwise fall back
    // to the defaults (page 0 = verb list, page 1 = variable list).
    int y = y_ + 8;                                  // top margin (tune later)
    const char *header = verbListHeader_.empty()
        ? ((verbListPage_ == 0) ? "VERB LIST" : "VARIABLE LIST")
        : verbListHeader_.c_str();
    verbListFont_->drawText(header, x_ + 10, y, green, scale);
    y += lineHeight;

    // Separator row: a dim-green divider line between the header and the list.
    // gfxPrimitives renderer calls take a packed 0xAARRGGBB pixel value.
    const Uint32 dividerPixel = 0xFF008000;   // opaque, (r=0, g=128, b=0)
    hlineColor(r_, x_ + 10, x_ + w_ - 10, y + lineHeight - 6, dividerPixel);
    y += lineHeight;

    // Up to 19 content lines fit below the separator; show a "..." marker if truncated.
    const int maxLines = 19;
    const int total = (int)verbListLines_.size();
    int n = total;
    if (n > maxLines) n = maxLines - 1;   // reserve one line for the "..." marker

    for (int i = 0; i < n; ++i) {
        verbListFont_->drawText(verbListLines_[i], x_ + 10, y, green, scale);
        y += lineHeight;
    }

    if (total > maxLines) {
        verbListFont_->drawText("...", x_ + 10, y, green, scale);
    }
}
  