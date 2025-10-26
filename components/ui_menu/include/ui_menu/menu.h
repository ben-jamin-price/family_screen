#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "display_panel/display.h"
#include "touch_gt9xx/touch_gt9xx.h"

/** Draw the 2×2 top-level menu into fb (RGB565, size w*h). */
void menu_draw(display_handle_t d, uint16_t *fb);

/**
 * Blocking interaction loop:
 *  - wait for touch
 *  - highlight tile
 *  - run demo
 *  - redraw menu
 */
void menu_loop(display_handle_t d, uint16_t *fb, touch_handle_t t);

#ifdef __cplusplus
} // extern "C"
#endif
