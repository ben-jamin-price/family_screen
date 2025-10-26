#include "ui_menu/menu.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "ui_gfx/ui_draw.h"
#include "display_panel/display.h"
#include "touch_gt9xx/touch_gt9xx.h"
#include "demos/demos.h"
#include "util/timing.h"

// static const char *TAG = "menu";

/* --- Colors (RGB565) --- */
#define RGB565(r,g,b)   (uint16_t)((((r)&0x1F)<<11) | (((g)&0x3F)<<5) | ((b)&0x1F))
#define C_BG            RGB565(0x02,0x04,0x02)
#define C_TILE          RGB565(0x06,0x0C,0x08)
#define C_TILE_HI       RGB565(0x08,0x12,0x0C)
#define C_FRAME         RGB565(0x0F,0x1F,0x0F)
#define C_TEXT          RGB565(0x1F,0x3F,0x1F)
#define C_CROSS         RGB565(0x1F,0x00,0x00)

typedef struct {
    int x0,y0,x1,y1;
    const char *label;
} tile_t;

static inline int clampi(int v, int lo, int hi) { return v<lo?lo:(v>hi?hi:v); }
static inline int inside(int x,int y,const tile_t* r) { return (x>=r->x0 && x<=r->x1 && y>=r->y0 && y<=r->y1); }

/* Layout scales with display size (portrait). */
static void build_tiles(display_handle_t d, tile_t out[4])
{
    const int W = display_width(d);
    const int H = display_height(d);

    const int gutter = (W < 480) ? 8 : 16;
    const int colw   = (W - gutter*3) / 2;
    const int rowh   = (H - gutter*3) / 2;

    // Row 0
    out[0] = (tile_t){ .x0=gutter, .y0=gutter, .x1=gutter+colw, .y1=gutter+rowh, .label="Color Bars" };
    out[1] = (tile_t){ .x0=gutter*2+colw, .y0=gutter, .x1=gutter*2+colw*2, .y1=gutter+rowh, .label="Gradient" };
    // Row 1
    out[2] = (tile_t){ .x0=gutter, .y0=gutter*2+rowh, .x1=gutter+colw, .y1=gutter*2+rowh*2, .label="Bounce 5s" };
    out[3] = (tile_t){ .x0=gutter*2+colw, .y0=gutter*2+rowh, .x1=gutter*2+colw*2, .y1=gutter*2+rowh*2, .label="Sleep/Wake" };
}

static void draw_frame(uint16_t *fb, int W,int H)
{
    ui_fill_rect565(fb, W,H, 0,0, W-1,H-1, C_BG);
    ui_draw_hline565(fb, W,H, 0, W-1, 0,     C_FRAME);
    ui_draw_hline565(fb, W,H, 0, W-1, H-1,   C_FRAME);
    ui_draw_vline565(fb, W,H, 0,     0, H-1, C_FRAME);
    ui_draw_vline565(fb, W,H, W-1,   0, H-1, C_FRAME);
}

static void draw_tile(uint16_t *fb, int W,int H, const tile_t* t, bool highlight)
{
    const uint16_t fill = highlight ? C_TILE_HI : C_TILE;
    ui_fill_rect565(fb, W,H, t->x0, t->y0, t->x1, t->y1, fill);

    // inner border
    ui_draw_hline565(fb, W,H, t->x0, t->x1, t->y0, C_FRAME);
    ui_draw_hline565(fb, W,H, t->x0, t->x1, t->y1, C_FRAME);
    ui_draw_vline565(fb, W,H, t->x0, t->y0, t->y1, C_FRAME);
    ui_draw_vline565(fb, W,H, t->x1, t->y0, t->y1, C_FRAME);

    // centered label
    const int text_w = (int)strlen(t->label) * (5 + 1);
    const int text_h = 7;
    const int cx = (t->x0 + t->x1) / 2;
    const int cy = (t->y0 + t->y1) / 2;
    const int x  = clampi(cx - text_w/2, t->x0+4, t->x1-4);
    const int y  = clampi(cy - text_h/2, t->y0+4, t->y1-4);
    (void)ui_draw_text5x7(fb, W,H, x, y, t->label, C_TEXT, UI_GFX_BG_TRANSPARENT);
}

void menu_draw(display_handle_t d, uint16_t *fb)
{
    const int W = display_width(d);
    const int H = display_height(d);

    tile_t tiles[4];
    build_tiles(d, tiles);

    draw_frame(fb, W,H);
    for (int i = 0; i < 4; ++i) draw_tile(fb, W,H, &tiles[i], false);
}

/* Wait for a press (simple polling with timeout) */
static bool wait_for_touch_first(touch_handle_t t, uint16_t *x, uint16_t *y, uint32_t timeout_ms)
{
    const uint64_t t0 = esp_timer_get_time();
    while (((esp_timer_get_time() - t0) / 1000ULL) < timeout_ms) {
        if (touch_gt9xx_read_first(t, x, y)) return true;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return false;
}

/* Drain touches until we observe 'quiet_ms' with no presses. */
static void touch_drain_until_quiet(touch_handle_t t, uint32_t quiet_ms)
{
    // ESP_LOGI(TAG, "drain: waiting for %u ms quiet", (unsigned)quiet_ms);
    uint32_t stable_ms = 0;
    uint64_t last = esp_timer_get_time();
    while (stable_ms < quiet_ms) {
        uint16_t x, y;
        bool pressed = touch_gt9xx_read_first(t, &x, &y);
        vTaskDelay(pdMS_TO_TICKS(10));
        uint64_t now = esp_timer_get_time();
        uint32_t dt  = (uint32_t)((now - last)/1000ULL);
        last = now;

        if (pressed) {
            if (dt > 50) dt = 50;
            stable_ms = 0;
        } else {
            if (dt > 50) dt = 50;
            stable_ms += dt;
        }
    }
    // ESP_LOGI(TAG, "drain: quiet");
}

/* Accept only if:
 *  - we never left the tile while pressed, and
 *  - release occurred with the last in-bounds position inside.
 */
static bool confirm_release_inside(display_handle_t d, uint16_t *fb,
                                   const tile_t *tile, touch_handle_t t,
                                   uint16_t start_x, uint16_t start_y)
{
    const int W = display_width(d);
    const int H = display_height(d);

    bool highlighted   = true;
    bool cancelled     = false;
    bool ever_pressed  = false;
    int  last_x        = (int)start_x;
    int  last_y        = (int)start_y;

    TickType_t wake = xTaskGetTickCount();
    const TickType_t poll_ticks = pdMS_TO_TICKS(20);

    for (;;) {
        uint16_t x, y;
        bool pressed = touch_gt9xx_read_first(t, &x, &y);

        if (pressed) {
            ever_pressed = true;
            last_x = (int)x;
            last_y = (int)y;

            const bool now_inside = inside(last_x, last_y, tile);

            if (!now_inside) {
                cancelled = true;
                if (highlighted) {
                    draw_tile(fb, W, H, tile, false);
                    (void)display_draw_bitmap(d, 0, 0, W, H, fb);
                    highlighted = false;
                }
            } else if (!cancelled && !highlighted) {
                draw_tile(fb, W, H, tile, true);
                (void)display_draw_bitmap(d, 0, 0, W, H, fb);
                highlighted = true;
            }

        } else if (ever_pressed) {
            const bool final_inside = inside(last_x, last_y, tile);
            if (!cancelled && final_inside) {
                return true;
            } else {
                if (highlighted) {
                    draw_tile(fb, W, H, tile, false);
                    (void)display_draw_bitmap(d, 0, 0, W, H, fb);
                    highlighted = false;
                }
                return false;
            }
        }

        vTaskDelayUntil(&wake, poll_ticks);
    }
}

void menu_loop(display_handle_t d, uint16_t *fb, touch_handle_t t)
{
    const int W = display_width(d);
    const int H = display_height(d);

    tile_t tiles[4];

    for (;;) {
        touch_drain_until_quiet(t, 60);

        build_tiles(d, tiles);
        menu_draw(d, fb);
        (void)display_draw_bitmap(d, 0,0, W,H, fb);

        uint16_t tx=0, ty=0;
        if (!wait_for_touch_first(t, &tx, &ty, 30000)) {
            continue;
        }

        ui_draw_crosshair(fb, W,H, tx, ty, (W < 480) ? 6 : 10, C_CROSS);
        (void)display_draw_bitmap(d, 0,0, W,H, fb);

        int chosen = -1;
        for (int i = 0; i < 4; ++i) {
            if (inside(tx, ty, &tiles[i])) { chosen = i; break; }
        }
        if (chosen < 0) {
            touch_drain_until_quiet(t, 80);
            continue;
        }

        draw_tile(fb, W,H, &tiles[chosen], true);
        (void)display_draw_bitmap(d, 0,0, W,H, fb);

        const bool accepted = confirm_release_inside(d, fb, &tiles[chosen], t, tx, ty);
        if (!accepted) {
            menu_draw(d, fb);
            (void)display_draw_bitmap(d, 0,0, W,H, fb);
            continue;
        }

        switch (chosen) {
            case 0: demo_color_bars(d, fb, 2);        break;
            case 1: demo_vertical_gradient(d, fb, 2); break;
            case 2: demo_bounce_seconds(d, fb, 5);    break;
            case 3: demo_checker_sleep_wake(d, fb, 2);break;
            default: break;
        }

        touch_drain_until_quiet(t, 80);
    }
}
