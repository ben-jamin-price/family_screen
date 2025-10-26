#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Special value: leave background pixels untouched */
#define UI_GFX_BG_TRANSPARENT 0x1FFFFu

/* Low-level pixel write (RGB565) with clipping. */
void ui_put_pixel565(uint16_t *fb, int w, int h, int x, int y, uint16_t rgb565);

/* Horizontal and vertical lines (inclusive end). */
void ui_draw_hline565(uint16_t *fb, int w, int h, int x0, int x1, int y, uint16_t rgb565);
void ui_draw_vline565(uint16_t *fb, int w, int h, int x, int y0, int y1, uint16_t rgb565);

/* Fill rectangle [x0..x1] × [y0..y1], inclusive, with clipping. */
void ui_fill_rect565(uint16_t *fb, int w, int h, int x0, int y0, int x1, int y1, uint16_t rgb565);

/* Draw one 5×7 character at (x,y). If bg==UI_GFX_BG_TRANSPARENT, background is preserved. */
void ui_draw_char5x7(uint16_t *fb, int w, int h, int x, int y, char c, uint16_t fg, uint32_t bg);

/* Draw null-terminated ASCII text with 1-px inter-glyph spacing. Returns end-x. */
int ui_draw_text5x7(uint16_t *fb, int w, int h, int x, int y, const char *s, uint16_t fg, uint32_t bg);

/* Simple crosshair centered at (cx,cy) with radius r. */
void ui_draw_crosshair(uint16_t *fb, int w, int h, int cx, int cy, int r, uint16_t rgb565);

#ifdef __cplusplus
} // extern "C"
#endif
