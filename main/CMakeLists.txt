# Main IDF component. With CONFIG_AUTOSTART_ARDUINO=y, the Arduino
# framework spawns its own task that calls setup() then loop() — we
# don't need to write app_main().
#
# rgb_db.cpp is the host-side double-buffered RGB-panel wrapper
# (Arduino_ESP32RGBPanelDB + RGBDisplayDB). It lives here in main/
# because it's host-board-specific, not a reusable library.
idf_component_register(
    SRCS
        "main.cpp"
        "rgb_db.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        # Arduino layer (added via idf_component.yml) gives us Serial,
        # delay, LittleFS, SD, Preferences, WiFi, etc. The component
        # manager registers it as managed_components/espressif__arduino-esp32;
        # we wrap that with the local components/arduino/ shim so we
        # can reference it as the short name "arduino".
        arduino
        # Arduino_GFX, NimBLE-Arduino, and the 4 legacy host-shared
        # libraries (TI_Extended_Basic_Interpreter, BleHidHost,
        # ESP32_File_Handling, TI_Font) get wired in by the NEXT commit
        # as IDF-component wrappers. Build is intentionally not green
        # at this commit — that's step 3.
)
