# Wraps JonVogel/ESP32_File_Handling (header-only) as an IDF component.
# file_io.h + dsk_image.h reference fs::FS / LittleFS / SD types, so
# downstream consumers get those through arduino.

set(FH_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../ESP32_File_Handling)

idf_component_register(
    INCLUDE_DIRS
        ${FH_ROOT}
    REQUIRES
        arduino
)
