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
        # The 4 legacy host-shared libraries, each wrapped as an IDF
        # component by a CMakeLists.txt under components/ that points
        # back into the submodule dir at repo root. Submodules stay
        # untouched. See components/<name>/CMakeLists.txt for each.
        interpreter        # TI_Extended_Basic_Interpreter (BASIC engine)
        ble-hid-host       # BleHidHost (NimBLE multi-peer HID host)
        file-handling      # ESP32_File_Handling (LittleFS / SD / DSK)
        ti-font            # TI_Font (8x8 ROM font + logo)
        # Arduino_GFX and NimBLE-Arduino are added in steps 4 and 5.
        # Build is still expected to fail at this commit on missing
        # Arduino_GFX_Library.h and NimBLE-Arduino headers.
)
