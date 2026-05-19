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
        # Third-party Arduino libraries (as IDF components under
        # components/). JonVogel forks because upstream has no IDF
        # support; the GFX fork only adds a CMakeLists.txt wrapper,
        # the NimBLE-Arduino fork bundles the NimBLE stack to avoid a
        # double-init collision with IDF's own NimBLE host (gotcha
        # table in transfer.md).
        Arduino_GFX
        NimBLE-Arduino
)

# IDF's toolchain promotes several warnings to errors that arduino-cli
# leaves as warnings. We demote them here for the main translation
# unit (which transitively pulls in the legacy submodule headers).
#
#   -Wreorder              ExecManager() initializers vs declaration order
#                          in TI_Extended_Basic_Interpreter/exec_manager.h
#   -Wformat-truncation    snprintf "%s" formats in token_parser.h and a
#                          handful of message buffers in main.cpp where
#                          worst-case input could exceed the destination
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=reorder
    -Wno-error=format-truncation
    # Arduino_GFX_Library.h (included by main.cpp) defines
    # `virtual void Arduino_GFX::flush(bool)`, which "hides" Print::flush()
    # in the base class chain. arduino-esp32 v2 didn't error on this
    # via arduino-cli's flags; IDF's -Werror=overloaded-virtual does.
    -Wno-error=overloaded-virtual
)
