# display_arduino_gfx_sunton — 8048S043C RGB parallel panel driver.
# See display_sunton.h for the module contract.
#
# One of a family of `display_XXX` components: each owns the tft
# object for its panel, provides an init() function main.cpp calls,
# and fills in the TiDisplay hooks host_common dispatches through.
idf_component_register(
    SRCS
        "src/display_sunton.cpp"
        # Local fork of Arduino_GFX's RGB panel that enables the
        # ESP_LCD double_fb mode (see rgb_db.h for the rationale
        # around bounce buffers + vsync-locked buffer swap).
        "src/rgb_db.cpp"
    INCLUDE_DIRS
        "include"
    REQUIRES
        host_common
        # Arduino_GFX_Library is where the panel driver base classes
        # live. Kept as REQUIRES (not PRIV) because main.cpp still
        # calls tft-> directly for the boot screen + drawStatusText,
        # which pulls in Arduino_GFX headers via display_sunton.h.
        Arduino_GFX
        arduino
        # esp_lcd_panel_rgb_t comes from esp_lcd; rgb_db uses it for
        # the double-buffered surface.
        esp_lcd
)
