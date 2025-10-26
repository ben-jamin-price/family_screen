#include "demos/demos.h"
#include "esp_timer.h"
#include "util/timing.h"

void demo_vertical_gradient(display_handle_t d, uint16_t *fb, int seconds)
{
    const int W = display_width(d);
    const int H = display_height(d);
    if (W <= 0 || H <= 0 || !fb) return;

    uint64_t t_end = esp_timer_get_time() + (uint64_t)seconds * 1000000ULL;

    for (int y = 0; y < H; ++y) {
        /* 6-bit green ramp; same mapping as original. */
        const uint8_t  g = (uint8_t)((y * 63) / (H - 1));
        const uint16_t c = ((uint16_t)g & 0x3F) << 5;
        uint16_t *row = fb + y * W;
        for (int x = 0; x < W; ++x) row[x] = c;
    }
    (void)display_draw_bitmap(d, 0, 0, W, H, fb);
    timing_sleep_until_abs_us(t_end);
}
