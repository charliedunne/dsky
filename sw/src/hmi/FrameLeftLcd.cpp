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

    spare2 = new AlarmLabel(r_, w_ / 2 + 1, 2 * (h_ / 2) / 3 + 2,
                            w_ / 2, (h_ / 2) / 3, std::string("NONE"));

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
    delete spare2;
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
    spare2->draw();
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

void FrameLeftLcd::setVerbList(const LeftLcdData &d)
{
    verbListMode_ = d.lVerbList;
    verbListLines_.clear();
    for (int i = 0; i < d.lVerbListCount && i < 20; ++i) {
        verbListLines_.push_back(std::string(d.lVerbListLines[i]));
    }
}

void FrameLeftLcd::drawVerbList()
{
    if (verbListMode_ != DRAW_ON || verbListLines_.empty()) return;

    // Draw over the grid background, stacked from the top.
    // scale ~0.5 fits the ~32px Gorton bitmap into the 335px-wide panel.
    const float scale = 0.5f;
    const int lineHeight = (int)(32 * scale) + 6;   // glyph height + gap (tune later)
    int y = y_ + 8;                                  // top margin (tune later)

    for (const std::string &line : verbListLines_) {
        verbListFont_->drawText(line, x_ + 10, y, Color(0, 255, 0, 255), scale);
        y += lineHeight;
    }
}
  