# Main IDF component. With CONFIG_AUTOSTART_ARDUINO=y, the Arduino
# framework spawns its own task that calls setup() then loop() — we
# don't need to write app_main().
#
# main.cpp is a placeholder stub for the scaffolding-only commit. The
# real .ino → .cpp port lands in the next commit, which is when the
# helper headers (rgb_db, sd_manager, ble_*) and the source moves
# happen too.
idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        # Arduino layer (added via idf_component.yml) gives us Serial,
        # delay, LittleFS, SD, Preferences, WiFi, etc. The component
        # manager registers it as managed_components/espressif__arduino-esp32;
        # we wrap that with the local components/arduino/ shim so we
        # can reference it as the short name "arduino".
        arduino
)
