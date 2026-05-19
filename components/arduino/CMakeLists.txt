# Shim component. The IDF Component Manager registers arduino-esp32
# under its namespaced name `espressif__arduino-esp32`, which is awkward
# to reference from every `PRIV_REQUIRES`. This thin alias lets the rest
# of the build write `REQUIRES arduino` instead.
idf_component_register(
    REQUIRES espressif__arduino-esp32
)
