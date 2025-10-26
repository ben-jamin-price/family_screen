idf_component_register(
    SRCS
        "src/board.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        # keep public surface clean
    PRIV_REQUIRES
        driver
        esp_timer
        esp32_p4_nano
)
