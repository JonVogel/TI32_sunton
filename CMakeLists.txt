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

# IDF v5.5's toolchain promotes -Wno-overloaded-virtual to -Werror by
# default. Multiple Arduino-style libraries we depend on (NimBLE-Arduino,
# Arduino_GFX, possibly AsyncTCP / ESPAsyncWebServer) define methods
# that hide same-named overloads in arduino-esp32's Print/Stream base
# classes — Print::flush, Print::availableForWrite, etc.
#
# idf_build_set_property(... APPEND) is the IDF-idiomatic way to add a
# compile flag to every component built by the project. add_compile_options
# alone doesn't propagate to IDF components because they set up their
# own per-component flag lists. Must be called AFTER project().
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=overloaded-virtual" APPEND)

# AsyncTCP's library defaults (in src/AsyncTCP.h, behind #ifndef) ship
# a 16 KB task stack and a 5-second ACK timeout. We tried 4 KB during
# the initial DRAM diet but the SPA response path (4 KB+ HTML through
# beginResponse_P) overflowed it. 8 KB is half the upstream default
# and the smallest size that handled the SPA without panicking.
# 3 s ACK timeout matches the responsiveness LAN clients expect.
target_compile_definitions(__idf_AsyncTCP PRIVATE
    CONFIG_ASYNC_TCP_STACK_SIZE=8192
    CONFIG_ASYNC_TCP_MAX_ACK_TIME=3000
)
