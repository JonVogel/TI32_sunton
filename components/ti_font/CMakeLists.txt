# Wraps JonVogel/TI_Font (header-only 8x8 TI ROM font + TI logo).
# Pure data tables; no Arduino dependency.

set(FONT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../TI_Font)

idf_component_register(
    INCLUDE_DIRS
        ${FONT_ROOT}
)
