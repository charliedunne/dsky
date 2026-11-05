#include "FrameLeftLcd.h"

// STL
#include <cmath>
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
    shutdown->setFgColor(Color::alarmYellow());

    // VERB 0 menu font (Gorton bitmap, scaled down at draw time)
    verbListFont_ = new Font(r_, GORTON_FONT_BITMAP, GORTON_FONT_FNT);

    // MOONPHASE result: percentage number below the moon disk + small "%" label.
    // Coordinates are window-absolute; the disk center sits at (x_+167, y_+205).
    // Digits are scaled to 80% of their default size (cell ~42x60 px), so the
    // right-aligned "NN" ink occupies abs x ~114..198; the "%" glyph is placed
    // to its right so that the combined row is centered on the panel.
    nMoon_ = new Number(r_, 3, x_ + 70, y_ + 368, Color::labelGreen(), false, 0.8f);

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
    delete nMoon_;
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

    // Draw alarm labels only when the VERB 0 menu or the MOONPHASE result is not shown
    if (verbListMode_ != DRAW_ON && moonMode_ != DRAW_ON) {
        drawLabels();
    }

    // Draw the separation lines (grid background), hidden while the VERB 0 menu or the
    // MOONPHASE result is shown, so the percentage number does not overlap the alarm grid
    if (verbListMode_ != DRAW_ON && moonMode_ != DRAW_ON) {
        drawLines();
    }

    // Draw the VERB 0 menu on top of the grid, if active
    if (verbListMode_ == DRAW_ON) {
        drawVerbList();
    }

    // Draw the MOONPHASE result number on top of the grid, if active.
    // No bookkeeping needed: the background fill + grid redraw each frame erases it.
    // The moon stays visible while the date is corrected (blinking X overlaid below).
    if (moonMode_ == DRAW_ON) {
        drawMoon();
    }

    // While the date is being corrected after a result was shown, blink an X over the
    // disk center to indicate the values are not valid yet. The previous moon image
    // remains visible underneath. The counter advances one frame per render; the X is
    // visible for the first half of DEFAULT_BLINKING_PERIOD frames (same convention as
    // the Draw base class).
    if (moonEditBlink_) {
        moonBlinkCounter_++;
        if ((moonBlinkCounter_ % DEFAULT_BLINKING_PERIOD) < (DEFAULT_BLINKING_PERIOD / 2)) {
            const int cx = x_ + 167;   // same center as the moon disk
            const int cy = y_ + 205;
            const int arm = (int)(112 * 0.7);   // X half-diagonal ~78 px, well inside the panel
            const Uint32 color = 0xFF00FF00;    // bright green
            thickLineColor(r_, cx - arm, cy - arm, cx + arm, cy + arm, 4, color);
            thickLineColor(r_, cx - arm, cy + arm, cx + arm, cy - arm, 4, color);
        }
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

void FrameLeftLcd::setMoon(DrawMode mode, int pct, float phase)
{
    moonMode_ = mode;
    moonPct_ = pct;
    moonPhase_ = phase;
    if (mode == DRAW_ON) {
        // Number right-aligns and blanks leading digits ("42" -> _ 4 2)
        nMoon_->setValue(std::to_string(pct));
    }

    // Enable/disable the number widget itself. The Draw base class starts
    // in DRAW_OFF, so without this draw() would always render blank digits.
    nMoon_->setMode(mode);
}

void FrameLeftLcd::setMoonEditBlink(bool on)
{
    if (on && !moonEditBlink_) {
        // Start the blink cycle in its visible half so the X is shown
        // immediately when editing begins. Edge-triggered: Hmi::update()
        // calls this every frame, so resetting on every true call would
        // freeze the counter and keep the X permanently visible.
        moonBlinkCounter_ = 0;
    }
    moonEditBlink_ = on;
}

void FrameLeftLcd::drawMoon()
{
    if (moonMode_ != DRAW_ON) return;

    const int R = 112;              // moon disk radius (px)
    const int cx = x_ + 167;        // disk center, horizontally centered on the panel
    const int cy = y_ + 205;        // shifted down to leave room for the title

    // Packed 0xAARRGGBB pixels for the gfxPrimitives calls.
    const Uint32 lit  = 0xFF00FF00; // bright green: illuminated part
    const Uint32 dim  = 0xFF004000; // dark green: unlit part of the disk
    const Uint32 edge = 0xFF00FF00; // disk outline so new moon stays visible

    // Title "MOON PHASE" centered at the top. estimateSize() reports the
    // unscaled size, so scale the width by tScale to center it on the panel.
    {
        int tw = 0, th = 0;
        verbListFont_->estimateSize("MOON PHASE", &tw, &th);
        const float tScale = 0.75f;
        const int tx = cx - (int)(tw * tScale) / 2;
        verbListFont_->drawText("MOON PHASE", tx, y_ + 14, Color::labelGreen(), tScale);
    }

    const float phi = moonPhase_;   // [0,1): 0=new, 0.5=full
    const bool waxing = (phi <= 0.5f);
    const double c = cos(2.0 * M_PI * phi);
    const int rx = (int)lround(R * fabs(c));

    // Retro scanline texture: instead of solid fills, paint the disk row by
    // row with horizontal lines every SCAN pixels. All three layers (dim
    // base, lit half-disk, terminator ellipse) share the same row selection
    // so they stay aligned and no band ever becomes a solid fill.
    const int SCAN = 3;
    for (int yy = cy - R; yy <= cy + R; ++yy) {
        if ((yy - (cy - R)) % SCAN != 0) continue;

        const double dy = yy - cy;
        const double t2 = 1.0 - (dy * dy) / (R * (double)R);
        if (t2 < 0.0) continue;
        const int dx = (int)lround(R * sqrt(t2));

        // Layer 1: dim base across the full disk width.
        hlineColor(r_, cx - dx, cx + dx, yy, dim);

        // Layer 2: lit half-disk (waxing -> right, waning -> left).
        if (waxing) {
            hlineColor(r_, cx, cx + dx, yy, lit);
        } else {
            hlineColor(r_, cx - dx, cx, yy, lit);
        }

        // Layer 3: terminator ellipse line; dim for crescents, lit for gibbous.
        if (rx > 0) {
            const int ex = (int)lround((double)rx * sqrt(t2));
            hlineColor(r_, cx - ex, cx + ex, yy,
                       (phi < 0.25f || phi > 0.75f) ? dim : lit);
        }
    }

    // Disk outline so the shape is visible even at new moon.
    circleColor(r_, cx, cy, R, edge);

    // Percentage number below the disk + small "%" label to its right.
    nMoon_->draw();

    // "%" glyph: the Gorton bitmap font has no "%" character, so draw it
    // manually with gfxPrimitives — two small rings plus a diagonal slash.
    const int pcx = x_ + 212;   // percent sign center, right of the digits
    const int pcy = y_ + 398;   // digit vertical middle (digits span abs y 372..432)
    circleColor(r_, pcx - 6, pcy - 10, 5, lit);
    circleColor(r_, pcx - 6, pcy - 10, 4, lit);
    circleColor(r_, pcx + 6, pcy + 10, 5, lit);
    circleColor(r_, pcx + 6, pcy + 10, 4, lit);
    thickLineColor(r_, pcx + 8, pcy - 13, pcx - 8, pcy + 13, 2, lit);
}
  