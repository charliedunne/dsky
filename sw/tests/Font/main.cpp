// Headless unit tests for the bitmap-font parser/renderer (sw/src/hmi/Font).
//
// The suite loads the real Gorton-Condensed assets (paths come from the
// generated config.h) and pins the behavior of parseFnt(), estimateSize()
// and drawText() against a software SDL renderer (dummy video driver).

#include "../common/test.h"

// Project headers (Font.h pulls in config.h, SDL2/SDL.h and Color.h)
#include "Font.h"
#include <SDL2/SDL_image.h>

// STL
#include <memory>
#include <functional>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cstdlib> // setenv()

// Headless SDL: dummy video driver, hidden window, software renderer.
// Initialized lazily on first use; lives until process exit so it always
// outlives any Font texture.
static SDL_Renderer *renderer() {
  static SDL_Renderer *ren = [] -> SDL_Renderer * {
    setenv("SDL_VIDEODRIVER", "dummy", 1);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return nullptr;
    if (IMG_Init(IMG_INIT_PNG) < 0) return nullptr;
    SDL_Window *win =
        SDL_CreateWindow("dsky-font-test", 0, 0, 800, 640, SDL_WINDOW_HIDDEN);
    if (win == nullptr) return nullptr;
    return SDL_CreateRenderer(win, -1, 0);
  }();
  return ren;
}

// A Font loaded from the real Gorton assets (paths from generated config.h).
static std::unique_ptr<Font> makeFont() {
  return std::make_unique<Font>(renderer(), GORTON_FONT_BITMAP, GORTON_FONT_FNT);
}

// Returns true iff fn() throws an exception of type T whose what() contains
// msgPart. (CHECK_THROWS cannot check the type or message.)
template <typename T>
static bool throws_as(std::function<void()> fn, const char *msgPart) {
  try {
    fn();
  } catch (const T &e) {
    return std::string(e.what()).find(msgPart) != std::string::npos;
  } catch (...) {
  }
  return false;
}

TEST(ctor_loads_real_font_and_estimates_hello) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  int w = -1, h = -1;
  font->estimateSize("HELLO", &w, &h);
  CHECK_EQ(w, 97); // H+E+L+L+O = 19+19+19+19+21
  CHECK_EQ(h, 34); // pixelHeight_ = max glyph height (Q)
}

TEST(estimate_size_letters_and_words) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  int w = -1, h = -1;
  font->estimateSize("HELLO WORLD", &w, &h);
  CHECK_EQ(w, 215); // 97 + space(11) + W+O+R+L+D = 27+21+20+19+20
  CHECK_EQ(h, 34);
}

TEST(estimate_size_digits) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  int w = -1, h = -1;
  font->estimateSize("0123456789", &w, &h);
  CHECK_EQ(w, 188); // 21+6+20+20+21+20+20+20+20+20
  CHECK_EQ(h, 34);
}

TEST(estimate_size_space_uses_spc_size) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  // Space uses spcSize_ = pixelHeight_/3 = 11, NOT the space glyph's own
  // width (which is 0 in this fnt).
  int w = -1, h = -1;
  font->estimateSize("A B", &w, &h);
  CHECK_EQ(w, 51); // A(21) + spcSize_(11) + B(19)
  CHECK_EQ(h, 34);
}

TEST(estimate_size_unknown_chars_ignored) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  // Lowercase letters have no glyphs in this font, so they are skipped
  // entirely by estimateSize().
  int w = -1, h = -1;
  font->estimateSize("aAbB", &w, &h);
  CHECK_EQ(w, 40); // A(21) + B(19); 'a' and 'b' ignored
  CHECK_EQ(h, 34);
}

TEST(estimate_size_newline_current_behavior) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  // KNOWN INCONSISTENCY: drawText() handles '\n' (new line) but
  // estimateSize() only does so when the fnt contains a glyph with id 10,
  // which this font lacks. We pin CURRENT behavior here: the newline is
  // ignored (no width or height contribution).
  int w = -1, h = -1;
  font->estimateSize("A\nB", &w, &h);
  CHECK_EQ(w, 40); // A(21) + B(19); '\n' ignored (no glyph id=10)
  CHECK_EQ(h, 34);
}

TEST(estimate_size_empty_string) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  int w = -1, h = -1;
  font->estimateSize("", &w, &h);
  CHECK_EQ(w, 0);
  CHECK_EQ(h, 34); // height is always at least pixelHeight_
}

TEST(draw_text_smoke_and_pixel_check) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  // Clear the target to black, then draw white text: a scaled string via
  // the Color overload and an unscaled one exercising the default-color
  // overload AND the '\n' path (second line at y+pixelHeight_).
  SDL_SetRenderDrawColor(renderer(), 0, 0, 0, 255);
  SDL_RenderClear(renderer());
  font->drawText("HELLO WORLD", 10, 10, Color(255, 255, 255, 255), 2.0f);
  font->drawText("A\nB", 400, 300, 1.0f);
  SDL_RenderPresent(renderer());

  // Read back the whole 800x640 target and count non-black pixels: the
  // white glyphs must have actually landed on the black background.
  SDL_Surface *shot =
      SDL_CreateRGBSurfaceWithFormat(0, 800, 640, 32, SDL_PIXELFORMAT_ARGB8888);
  CHECK(shot != nullptr);
  if (shot) {
    SDL_RenderReadPixels(renderer(), nullptr, SDL_PIXELFORMAT_ARGB8888,
                         shot->pixels, shot->pitch);
    int nonBlack = 0;
    for (int y = 0; y < 640; ++y) {
      const Uint32 *row = (const Uint32 *)(shot->pixels + y * shot->pitch);
      for (int x = 0; x < 800; ++x) {
        Uint32 px = row[x];
        if (((px >> 16) & 0xFFu) || ((px >> 8) & 0xFFu) || (px & 0xFFu))
          ++nonBlack;
      }
    }
    CHECK(nonBlack > 0); // white glyphs were actually rendered onto black
    SDL_FreeSurface(shot);
  }
}

TEST(ctor_missing_bitmap_throws_domain_error) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  // texture_ is NULL on this path; SDL_DestroyTexture(NULL) in the dtor is
  // a safe no-op.
  CHECK(throws_as<std::domain_error>(
      [] { Font f(renderer(), "/nonexistent/dsky-font-test.png", GORTON_FONT_FNT); },
      "Invalid Bitmap for Font"));
}

TEST(parse_fnt_unopenable_throws_runtime_error) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  // Valid bitmap, bogus fnt path.
  CHECK(throws_as<std::runtime_error>(
      [] { Font f(renderer(), GORTON_FONT_BITMAP, "/nonexistent/dsky-font-test.fnt"); },
      "Font::parseFnt: cannot open"));
}

TEST(parse_fnt_no_glyphs_throws_runtime_error) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  // Header-only fnt: no "char id=..." lines, so the parser must reject it.
  {
    std::ofstream out("/tmp/opencode/dsky_font_empty.fnt");
    out << "info face=\"Test\" size=32\n"
        << "common lineHeight=32 base=28 scaleW=16 scaleH=16 pages=1 packed=0\n"
        << "page id=0 file=\"test.png\"\n"
        << "chars count=0\n";
  }
  CHECK(throws_as<std::runtime_error>(
      [] { Font f(renderer(), GORTON_FONT_BITMAP, "/tmp/opencode/dsky_font_empty.fnt"); },
      "Font::parseFnt: no glyphs parsed"));
}

// Regression: the '*' (id=42) glyph must be parsed from the Gorton .fnt.
// estimateSize counts a char only when getW/getH both resolve, using the
// cell width (not xadvance). A=21, *=20, B=19 => w=60 if stored, 40 if not.
TEST(asterisk_glyph_parsed_by_estimate_size) {
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;
  int w = -1, h = -1;
  font->estimateSize("A*B", &w, &h);
  CHECK(w == 60);
  int w2 = -1, h2 = -1;
  font->estimateSize("*", &w2, &h2);
  CHECK(w2 == 20);
}

// Regression: drawText must actually blit the '*' cell. Draws at pen (10,10)
// scale 1.0 => dest x[10..30) y[10..42); counts non-black pixels (~287).
TEST(asterisk_glyph_renders_pixels) {
  CHECK(renderer() != nullptr);
  if (renderer() == nullptr) return;
  auto font = makeFont();
  CHECK(font != nullptr);
  if (!font) return;

  SDL_SetRenderDrawColor(renderer(), 0, 0, 0, 255);
  SDL_RenderClear(renderer());
  font->drawText("*", 10, 10, Color(255, 255, 255, 255), 1.0f);
  SDL_RenderPresent(renderer());

  SDL_Surface *shot =
      SDL_CreateRGBSurfaceWithFormat(0, 800, 640, 32, SDL_PIXELFORMAT_ARGB8888);
  CHECK(shot != nullptr);
  if (shot) {
    SDL_RenderReadPixels(renderer(), nullptr, SDL_PIXELFORMAT_ARGB8888, shot->pixels, shot->pitch);
    int lit = 0;
    for (int y = 10; y < 42; ++y) {
      const Uint32 *row = (const Uint32 *)(shot->pixels + y * shot->pitch);
      for (int x = 10; x < 30; ++x) {
        Uint32 px = row[x];
        if (((px >> 16) & 0xFFu) || ((px >> 8) & 0xFFu) || (px & 0xFFu)) ++lit;
      }
    }
    CHECK(lit > 100);
    SDL_FreeSurface(shot);
  }
}

TESTS_MAIN;
