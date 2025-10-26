#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "demos/demos.h"
#include "esp_timer.h"
#include "util/timing.h"

void demo_checker_sleep_wake(display_handle_t d, uint16_t *fb, int seconds)
{
    const int W = display_width(d);
    const int H = display_height(d);
    if (W <= 0 || H <= 0 || !fb) return;

    uint64_t t_end = esp_timer_get_time() + (uint64_t)seconds * 1000000ULL;

    /* Checkerboard fill */
    const int sz = 32;
    for (int y = 0; y < H; ++y) {
        uint16_t *row = fb + y * W;
        for (int x = 0; x < W; ++x) {
            row[x] = (((x / sz) ^ (y / sz)) & 1) ? 0xFFFF : 0x0000;
        }
    }
    (void)display_draw_bitmap(d, 0, 0, W, H, fb);

    /* Short sleep→wake cycle */
    vTaskDelay(pdMS_TO_TICKS(300));
    (void)display_on(d, false);
    vTaskDelay(pdMS_TO_TICKS(300));
    (void)display_on(d, true);
    vTaskDelay(pdMS_TO_TICKS(300));

    timing_sleep_until_abs_us(t_end);
}
