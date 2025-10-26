#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "demos/demos.h"
#include "util/timing.h"

typedef struct { int x, y, dx, dy, size; uint16_t color; } Sprite;

/* Inner fill (RGB565) with simple clipping. */
static inline void fill_rect565(uint16_t *fb, int W, int H, int x0,int y0,int x1,int y1, uint16_t c)
{
    if (!fb) return;
    if (x0 < 0) 
        x0 = 0; 
    if (y0 < 0) 
        y0 = 0;
    if (x1 > W) 
        x1 = W; 
    if (y1 > H) 
        y1 = H;
    for (int y = y0; y < y1; ++y) {
        uint16_t *row = fb + y * W;
        for (int x = x0; x < x1; ++x) row[x] = c;
    }
}

static inline void copy_region_to_buf(const uint16_t *fb, int W, int x0,int y0,int x1,int y1, uint16_t *out)
{
    const int rw = x1 - x0;
    for (int y = y0; y < y1; ++y) {
        const uint16_t *src = fb + y * W + x0;
        memcpy(out + (y - y0) * rw, src, rw * sizeof(uint16_t));
    }
}

void demo_bounce_seconds(display_handle_t d, uint16_t *fb, int seconds)
{
    const int W = display_width(d);
    const int H = display_height(d);
    if (W <= 0 || H <= 0 || !fb) return;

    /* Clear + present once (matches original cadence). */
    memset(fb, 0, (size_t)W * H * sizeof(uint16_t));
    (void)display_draw_bitmap(d, 0, 0, W, H, fb);
    vTaskDelay(pdMS_TO_TICKS(30));

    Sprite s = { .x = 20, .y = 20, .dx = 6, .dy = 5, .size = 80, .color = 0xF800 };
    const int margin = 2, max_w = 128, max_h = 128;

    /* Prefer internal/8-bit capable buffer; fall back to malloc. */
    uint16_t *rect_buf = heap_caps_malloc((size_t)max_w * max_h * sizeof(uint16_t),
                                          MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!rect_buf) rect_buf = (uint16_t *)malloc((size_t)max_w * max_h * sizeof(uint16_t));

    const uint64_t t_end = esp_timer_get_time() + (uint64_t)seconds * 1000000ULL;
    uint64_t next_frame = esp_timer_get_time();
    const uint32_t frame_us = 24500; // ~40.8 FPS

    while (esp_timer_get_time() < t_end) {
        /* erase old */
        fill_rect565(fb, W, H, s.x, s.y, s.x + s.size, s.y + s.size, 0x0000);

        /* advance & bounce */
        s.x += s.dx; s.y += s.dy;
        if (s.x < 0 || s.x + s.size > W) { s.dx = -s.dx; s.x += s.dx; }
        if (s.y < 0 || s.y + s.size > H) { s.dy = -s.dy; s.y += s.dy; }

        /* draw new */
        fill_rect565(fb, W, H, s.x, s.y, s.x + s.size, s.y + s.size, s.color);

        /* dirty region around sprite (with margin) */
        int x0 = s.x - margin, y0 = s.y - margin;
        int x1 = s.x + s.size + margin, y1 = s.y + s.size + margin;
        if (x0 < 0) 
            x0 = 0; 
        if (y0 < 0) 
            y0 = 0;
        if (x1 > W) 
            x1 = W; 
        if (y1 > H) 
            y1 = H;

        const int rw = x1 - x0, rh = y1 - y0;
        if (rect_buf && rw <= max_w && rh <= max_h) {
            copy_region_to_buf(fb, W, x0, y0, x1, y1, rect_buf);
            (void)display_draw_bitmap(d, x0, y0, x1, y1, rect_buf);
        } else {
            (void)display_draw_bitmap(d, 0, 0, W, H, fb);
        }

        timing_sleep_periodic_us(&next_frame, frame_us);
    }

    if (rect_buf) free(rect_buf);
}
