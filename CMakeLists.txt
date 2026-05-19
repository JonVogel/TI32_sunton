cmake_minimum_required(VERSION 3.16)

# Local components live under components/. For now that's just the
# `arduino` shim (so downstream code can `REQUIRES arduino` instead of
# the awkward `espressif__arduino-esp32`). More component dirs will be
# added in later commits as we move source out of root into the IDF
# layout.
set(EXTRA_COMPONENT_DIRS components)

# 8048S043C has a CH340 USB-UART bridge wired to UART0 (NOT native
# USB-Serial-JTAG like the Box-3), so Serial.print routes through the
# bridge. ARDUINO_USB_CDC_ON_BOOT=0 keeps Serial on UART0.
add_compile_definitions(
    ARDUINO_USB_CDC_ON_BOOT=0
    ARDUINO_USB_MODE=1
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ti-extended-basic-esp32)
