# Main IDF component. With CONFIG_AUTOSTART_ARDUINO=y, the Arduino
# framework spawns its own task that calls setup() then loop() — we
# don't need to write app_main().
#
# The 8048S043C panel driver + rgb_db.cpp double-buffered RGB wrapper
# now live in components/display_arduino_gfx_sunton — main.cpp just
# calls display_arduino_gfx_sunton::init() and uses the extern tft
# pointer for its remaining direct calls (boot screen + status bar).
idf_component_register(
    SRCS
        "main.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        # Arduino layer (added via idf_component.yml) gives us Serial,
        # delay, LittleFS, SD, Preferences, WiFi, etc. The component
        # manager registers it as managed_components/espressif__arduino-esp32;
        # we wrap that with the local components/arduino/ shim so we
        # can reference it as the short name "arduino".
        arduino
        # The host-shared layers come from the consolidated
        # TI_EB_Emulator_ESP submodule (components/ti-emulator/). Its
        # own components/ subdir is added to EXTRA_COMPONENT_DIRS at
        # the top-level CMakeLists, so each one is referenced by name
        # here. Same bundle Box-3 uses — single source of truth, so a
        # fix in the interpreter or file_handling lands on both targets
        # via one submodule bump.
        interpreter        # TI Extended BASIC engine
        ble_hid_host       # NimBLE multi-peer BLE HID host
        file_handling      # LittleFS / SD / V9T9 DSK file I/O
        font               # 8x8 TI ROM font + TI logo (was: ti_font)
        host_common        # shared host layer (palette, resolveColor,
                           # eventually line editor + tiPrint family +
                           # boot screen + sprites + command dispatch)
        # Panel driver + tft globals — one of a family of
        # display_XXX components. Pull in a different one to target
        # a different panel; nothing else in main.cpp needs to change.
        display_arduino_gfx_sunton
        # Third-party Arduino libraries (as IDF components under
        # components/). JonVogel forks because upstream has no IDF
        # support; the GFX fork only adds a CMakeLists.txt wrapper,
        # the NimBLE-Arduino fork bundles the NimBLE stack to avoid a
        # double-init collision with IDF's own NimBLE host (gotcha
        # table in transfer.md). AsyncTCP / ESPAsyncWebServer power
        # the web file manager (web_files.cpp).
        Arduino_GFX
        NimBLE-Arduino
        AsyncTCP
        ESPAsyncWebServer
        # Shared HTTP file manager (JonVogel/TI_Web_Files_ESP). Brings
        # webfiles::init / tick / setBusy. The library has its own
        # transitive deps on AsyncTCP + ESPAsyncWebServer + arduino +
        # file_handling + interpreter, but PRIV_REQUIRES wants the
        # direct ones too so we list AsyncTCP/ESPAsyncWebServer above.
        web_files
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
