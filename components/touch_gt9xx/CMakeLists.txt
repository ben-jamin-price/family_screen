idf_component_register(
    SRCS
        "src/touch_gt9xx.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        board
    PRIV_REQUIRES
        driver       # i2c_master
        i2c_bus
        freertos
)
