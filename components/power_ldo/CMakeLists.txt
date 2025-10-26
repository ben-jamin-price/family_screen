idf_component_register(
    SRCS
        "src/power_ldo.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        # keep public surface clean
)
