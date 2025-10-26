#include <string.h>
#include "demos/demos.h"
#include "esp_timer.h"
#include "util/timing.h"

static inline void full_present(display_handle_t d, uint16_t *fb, int W, int H) {
    (void)display_draw_bitmap(d, 0, 0, W, H, fb);
}

void demo_color_bars(display_handle_t d, uint16_t *fb, int seconds)
{
    const int W = display_width(d);
    const int H = display_height(d);
    if (W <= 0 || H <= 0 || !fb) return;

    uint64_t t_end = esp_timer_get_time() + (uint64_t)seconds * 1000000ULL;

    /* Same 7-bar palette as before. */
    static const uint16_t bars[7] = {0xF800,0xFBE0,0x07E0,0x07FF,0x001F,0xF81F,0xFFFF};
    const int band = (W + 6) / 7;

    for (int y = 0; y < H; ++y) {
        uint16_t *row = fb + y * W;
        for (int x = 0; x < W; ++x) {
            int idx = x / band; if (idx > 6) idx = 6;
            row[x] = bars[idx];
        }
    }
    full_present(d, fb, W, H);
    timing_sleep_until_abs_us(t_end);
}
