#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "display_panel/display.h"

/* Draw 7 vertical color bars and hold for N seconds. */
void demo_color_bars(display_handle_t d, uint16_t *fb, int seconds);

/* Vertical green gradient (top→bottom), then hold for N seconds. */
void demo_vertical_gradient(display_handle_t d, uint16_t *fb, int seconds);

/* Animate a bouncing square for N seconds; uses partial blits when reasonable. */
void demo_bounce_seconds(display_handle_t d, uint16_t *fb, int seconds);

/* Draw checkerboard, briefly sleep the panel, then wake and hold until deadline. */
void demo_checker_sleep_wake(display_handle_t d, uint16_t *fb, int seconds);

#ifdef __cplusplus
} // extern "C"
#endif
