idf_component_register(
    SRCS
        "src/font5x7.c"
        "src/ui_draw.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        # keep public surface clean
)
