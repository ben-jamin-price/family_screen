idf_component_register(
    SRCS
        "src/fb.c"
        "src/timing.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        # keep public surface clean
    PRIV_REQUIRES
        freertos
        esp_timer
)
