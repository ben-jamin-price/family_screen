#include "ui_gfx/ui_draw.h"
#include "ui_gfx/font5x7.h"
#include <string.h>

static inline void swap_int(int *a, int *b) { int t=*a; *a=*b; *b=t; }

void ui_put_pixel565(uint16_t *fb, int w, int h, int x, int y, uint16_t rgb565)
{
    if (!fb) return;
    if ((unsigned)x >= (unsigned)w) return;
    if ((unsigned)y >= (unsigned)h) return;
    fb[y * w + x] = rgb565;
}

void ui_draw_hline565(uint16_t *fb, int w, int h, int x0, int x1, int y, uint16_t rgb565)
{
    if ((unsigned)y >= (unsigned)h) return;
    if (x0 > x1) swap_int(&x0, &x1);
    if (x1 < 0 || x0 >= w) return;
    if (x0 < 0) x0 = 0;
    if (x1 >= w) x1 = w - 1;
    uint16_t *p = &fb[y * w + x0];
    for (int x = x0; x <= x1; ++x) { *p++ = rgb565; }
}

void ui_draw_vline565(uint16_t *fb, int w, int h, int x, int y0, int y1, uint16_t rgb565)
{
    if ((unsigned)x >= (unsigned)w) return;
    if (y0 > y1) swap_int(&y0, &y1);
    if (y1 < 0 || y0 >= h) return;
    if (y0 < 0) y0 = 0;
    if (y1 >= h) y1 = h - 1;
    uint16_t *p = &fb[y0 * w + x];
    for (int y = y0; y <= y1; ++y) { *p = rgb565; p += w; }
}

void ui_fill_rect565(uint16_t *fb, int w, int h, int x0, int y0, int x1, int y1, uint16_t rgb565)
{
    if (x0 > x1) swap_int(&x0, &x1);
    if (y0 > y1) swap_int(&y0, &y1);
    if (x1 < 0 || y1 < 0 || x0 >= w || y0 >= h) return;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= w) x1 = w - 1;
    if (y1 >= h) y1 = h - 1;
    const int span = x1 - x0 + 1;
    for (int y = y0; y <= y1; ++y) {
        uint16_t *row = &fb[y * w + x0];
        for (int i = 0; i < span; ++i) row[i] = rgb565;
    }
}

void ui_draw_char5x7(uint16_t *fb, int w, int h, int x, int y, char c, uint16_t fg, uint32_t bg)
{
    const uint8_t *g = ui_gfx_font5x7_glyph(c);
    for (int col = 0; col < UI_GFX_FONT5x7_WIDTH; ++col) {
        const uint8_t bits = g[col];
        for (int row = 0; row < UI_GFX_FONT5x7_HEIGHT; ++row) {
            const bool on = (bits >> row) & 0x1;
            if (on) {
                ui_put_pixel565(fb, w, h, x + col, y + row, fg);
            } else if (bg != UI_GFX_BG_TRANSPARENT) {
                ui_put_pixel565(fb, w, h, x + col, y + row, (uint16_t)bg);
            }
        }
    }
    /* 1px spacing column on the right */
    if (bg != UI_GFX_BG_TRANSPARENT) {
        for (int row = 0; row < UI_GFX_FONT5x7_HEIGHT; ++row) {
            ui_put_pixel565(fb, w, h, x + UI_GFX_FONT5x7_WIDTH, y + row, (uint16_t)bg);
        }
    }
}

int ui_draw_text5x7(uint16_t *fb, int w, int h, int x, int y, const char *s, uint16_t fg, uint32_t bg)
{
    int cursor = x;
    for (const char *p = s; *p; ++p) {
        ui_draw_char5x7(fb, w, h, cursor, y, *p, fg, bg);
        cursor += UI_GFX_FONT5x7_WIDTH + 1;
    }
    return cursor;
}

void ui_draw_crosshair(uint16_t *fb, int w, int h, int cx, int cy, int r, uint16_t rgb565)
{
    if (r < 0) return;
    /* Cross arms + small center box for visibility. */
    ui_draw_hline565(fb, w, h, cx - r, cx + r, cy, rgb565);
    ui_draw_vline565(fb, w, h, cx, cy - r, cy + r, rgb565);
    ui_fill_rect565(fb, w, h, cx - 1, cy - 1, cx + 1, cy + 1, rgb565);
}
