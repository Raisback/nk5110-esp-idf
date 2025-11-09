# This file is placed in the 'components/nk5110_driver/' directory.

# Declares the source file and specifies the required ESP-IDF components.
# We are fixing dependency resolution issues for ESP-IDF v5.5.1 by using stable,
# high-level component names that are guaranteed to exist.
idf_component_register(
    SRCS "nk5110.c"
    INCLUDE_DIRS "."
    REQUIRES "driver" "esp_system" "freertos"
)
