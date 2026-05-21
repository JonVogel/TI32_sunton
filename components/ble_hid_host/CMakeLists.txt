# Wraps JonVogel/BleHidHost (header-only) as an IDF component.
# BleHidHost.h pulls NimBLE-Arduino types so REQUIRES adds it.

set(BHID_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../BleHidHost)

idf_component_register(
    INCLUDE_DIRS
        ${BHID_ROOT}
    REQUIRES
        arduino
        NimBLE-Arduino
)
