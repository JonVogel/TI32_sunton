// display_sunton.cpp — 8048S043C RGB parallel panel init + TiDisplay
// hook wrappers. Consolidates what used to live in TI32-sunton main.cpp:
// rgbBus definition, tft pointer, initDisplay, paintBorder, and the
// six panel-side host hook impls. See display_sunton.h for the
// module contract.

#include <Arduino.h>
#include "display_sunton.h"
#include "rgb_db.h"

namespace display_arduino_gfx_sunton
{

// 8048S043C geometry — 800x480 RGB parallel panel. TI grid 32x24 at
// 16x16 (2x scaled 8x8 font) → 512x384 grid centered at (144, 40)
// with an 8-px TI-style border ring outside.
static constexpr int  kCols            = 32;
static constexpr int  kRows            = 24;
static constexpr int  kCharW           = 16;
static constexpr int  kCharH           = 16;
static constexpr int  kScreenW         = 800;
static constexpr int  kScreenH         = 480;
static constexpr int  kDisplayXOffset  = (kScreenW - kCols * kCharW) / 2;
static constexpr int  kDisplayYOffset  = ((kScreenH - kRows * kCharH) / 2) - 8;
static constexpr int  kBorderW         = 8;
static constexpr int  kGridBottomY     = kDisplayYOffset + kRows * kCharH;
static constexpr int  kTftBl           = 2;

// RGB bus + tft — same pin config sunton used before the extraction.
// Kept file-static; main.cpp uses `tft` via the extern declared in
// display_sunton.h.
static Arduino_ESP32RGBPanelDB* s_rgbBus = new Arduino_ESP32RGBPanelDB(
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5  /* G0 */, 6  /* G1 */, 7  /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8  /* B0 */, 3  /* B1 */, 46 /* B2 */, 9  /* B3 */, 1  /* B4 */,
    0, 8, 4, 8,      // hsync: polarity, front porch, pulse width, back porch
    0, 8, 4, 8,      // vsync: polarity, front porch, pulse width, back porch
    1, 14000000,     // pclk_active_neg, prefer_speed (14 MHz)
    false,           // useBigEndian
    0, 0,            // de_idle_high, pclk_idle_high
    20 * 800         // bounce_buffer_size_px
);

RGBDisplayDB* tft = new RGBDisplayDB(800, 480, s_rgbBus);

// Paint an 8-px TI-style screen-color frame hugging the 32x24 grid.
// Reads tihost::bgColor for the border color. Public — main.cpp still
// calls this directly from gfxReset + tiSetScreenColor.
void paintBorder()
{
  int frameX = kDisplayXOffset - kBorderW;
  int frameY = kDisplayYOffset - kBorderW;
  int frameW = kCols * kCharW + 2 * kBorderW;
  tft->fillRect(frameX, frameY, frameW, kBorderW, tihost::bgColor);
  tft->fillRect(frameX, kGridBottomY, frameW, kBorderW, tihost::bgColor);
  tft->fillRect(frameX, kDisplayYOffset, kBorderW, kRows * kCharH,
                tihost::bgColor);
  int rightX = kDisplayXOffset + kCols * kCharW;
  tft->fillRect(rightX, kDisplayYOffset, kBorderW, kRows * kCharH,
                tihost::bgColor);
}

static void hostPushCell_impl(int px, int py, int w, int h,
                              const uint16_t* pixels)
{
  tft->draw16bitRGBBitmap(px, py, (uint16_t*)pixels, w, h);
}
static void hostFillRect_impl(int px, int py, int w, int h, uint16_t color)
{
  tft->fillRect(px, py, w, h, color);
}
static void hostPutPixel_impl(int px, int py, uint16_t color)
{
  tft->writePixel(px, py, color);
}
static void hostFillScreen_impl(uint16_t color)
{
  tft->fillScreen(color);
}
static void hostFlush_impl()
{
  tft->flush();
}
// Sunton's whole-panel background: black around, TI-cyan border ring,
// bg-color grid area. Panels with no border margin (box) trivialize
// this to fillScreen(bg).
static void hostFillBackground_impl(uint16_t bg)
{
  tft->fillScreen(0x0000);
  paintBorder();
  tft->fillRect(kDisplayXOffset, kDisplayYOffset, kCols * kCharW,
                kRows * kCharH, bg);
}

bool init(tihost::TiHostConfig& cfg, tihost::TiDisplay& display)
{
  // Keep backlight off until display is ready so we don't briefly
  // show garbage while the panel init runs.
  pinMode(kTftBl, OUTPUT);
  digitalWrite(kTftBl, LOW);

  tft->begin();
  // 180° rotation puts the SD-card slot / connectors at the top of
  // the visible image. RGB panels usually only honor rotation 0
  // natively; if 2 doesn't take, drawing looks unrotated and we can
  // fall back to app-level coordinate flipping.
  tft->setRotation(2);
  tft->fillScreen(0x0000);
  tft->flush();

  digitalWrite(kTftBl, HIGH);

  cfg.cols             = kCols;
  cfg.rows             = kRows;
  cfg.char_w           = kCharW;
  cfg.char_h           = kCharH;
  cfg.screen_w         = kScreenW;
  cfg.screen_h         = kScreenH;
  cfg.display_x_offset = kDisplayXOffset;
  cfg.display_y_offset = kDisplayYOffset;

  display.hostBegin          = nullptr;
  display.hostPushCell       = hostPushCell_impl;
  display.hostFillRect       = hostFillRect_impl;
  display.hostPutPixel       = hostPutPixel_impl;
  display.hostFillScreen     = hostFillScreen_impl;
  display.hostFlush          = hostFlush_impl;
  display.hostFillBackground = hostFillBackground_impl;
  display.hostPaintBorder    = paintBorder;
  // hostHonk / hostPostScroll / hostReadBleKey — per-host, filled
  // by main.cpp after init() returns.

  return true;
}

} // namespace display_arduino_gfx_sunton
