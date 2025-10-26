# "main" component: glue for board bring-up, panel init, touch menu, and demo fallback.
idf_component_register(
    SRCS
        "main.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        board          # carrier traits (lanes, resolution, touch pins)
        power_ldo      # MIPI PHY rail helper
        display_panel  # JD9365 panel wrapper
        touch_gt9xx    # GT911 init/read
        ui_menu        # 2×2 menu + interaction
        demos          # visual demos (color bars, gradient, etc.)
        util           # fb allocator, timing
    PRIV_REQUIRES
        freertos
)
