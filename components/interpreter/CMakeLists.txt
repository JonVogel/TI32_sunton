# Wraps the JonVogel/TI_Extended_Basic_Interpreter submodule (mounted at
# repo root) as an IDF component. The submodule is mostly header-only;
# ti_platform.cpp provides the weak yield/wifi/speech default hooks the
# host project optionally overrides.
#
# Submodule stays untouched — this wrapper sits outside it and points
# at the submodule path so the submodule remains a pure read-only
# reference to upstream.

set(INTERP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../TI_Extended_Basic_Interpreter)

idf_component_register(
    SRCS
        ${INTERP_ROOT}/ti_platform.cpp
    INCLUDE_DIRS
        ${INTERP_ROOT}
    REQUIRES
        # Headers reference Arduino types (String, Serial, etc.) so
        # downstream consumers need to see Arduino's headers
        # transitively via this component's REQUIRES.
        arduino
)
