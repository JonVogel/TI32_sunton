// display_sunton.h — public API for the 8048S043C Arduino_GFX RGB
// panel driver.
//
// Main.cpp calls display_arduino_gfx_sunton::init(cfg, display) once
// from setup(), which:
//   * Sets up the RGB parallel bus + double-buffered surface.
//   * Fills in the geometry portion of TiHostConfig (32x24 grid at
//     16x16 in an 800x480 panel, centered with a 16 px border).
//   * Wires the panel-specific TiDisplay hooks (hostPushCell,
//     hostFillRect, hostPutPixel, hostFillScreen, hostFlush,
//     hostPaintBorder, hostFillBackground).
//
// Feature-specific hooks (hostHonk — no audio, hostPostScroll,
// hostReadBleKey) stay in main.cpp.
//
// tft is exposed via `extern RGBDisplayDB* tft;` for now — sunton's
// boot screen + status bar still use it directly. Those move into
// host_common in later commits.

#pragma once

#include <Arduino_GFX_Library.h>
#include "ti_host.h"
// rgb_db.h defines Arduino_ESP32RGBPanelDB + RGBDisplayDB. Included
// publicly because main.cpp calls tft->flush(), tft->fillRect(), and
// tft->draw16bitRGBBitmap() directly from sprite/status code that
// hasn't been moved into host_common yet. Migrates to a private
// implementation detail once those code paths move.
#include "rgb_db.h"

namespace display_arduino_gfx_sunton
{

extern RGBDisplayDB* tft;

// One-shot init. Fills cfg with sunton geometry + populates the
// TiDisplay hooks. Returns false if the panel init fails (currently
// no observed failure path — the RGB peripheral init runs
// unconditionally).
bool init(tihost::TiHostConfig& cfg, tihost::TiDisplay& display);

// Paint the 8-px TI-style screen-color border ring hugging the 32x24
// char grid. Reads tihost::bgColor. Wired via TiDisplay.hostPaintBorder
// by init() — but also called directly from main.cpp code that still
// touches the panel (tiSetScreenColor, gfxReset) until those move.
void paintBorder();

} // namespace display_arduino_gfx_sunton
