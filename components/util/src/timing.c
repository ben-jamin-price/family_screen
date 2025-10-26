#include "util/timing.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

uint64_t timing_now_us(void)
{
    return (uint64_t)esp_timer_get_time();
}

static inline uint64_t now_ms_from_us(uint64_t us)
{
    return us / 1000ULL;
}

bool timing_sleep_until_abs_us(uint64_t deadline_us)
{
    int64_t now = (int64_t)esp_timer_get_time();
    int64_t remain = (int64_t)deadline_us - now;
    if (remain <= 0) return false;

    // Coarse RTOS sleep, leave ~1 tick for the precise finish.
    const int64_t us_per_tick = 1000000LL / configTICK_RATE_HZ;
    if (remain > us_per_tick) {
        TickType_t ticks = (TickType_t)(remain / us_per_tick);
        if (ticks > 1) {
            vTaskDelay(ticks - 1);
        }
        // Recompute precisely after waking
        now = (int64_t)esp_timer_get_time();
        remain = (int64_t)deadline_us - now;
    }

    // Short busy-wait to land near the target
    if (remain > 0) {
        if (remain > (int64_t)UINT32_MAX) remain = (int64_t)UINT32_MAX;
        esp_rom_delay_us((uint32_t)remain);
    }
    return true;
}

bool timing_sleep_until_abs_ms(uint64_t deadline_ms)
{
    const uint64_t now_ms = timing_now_us() / 1000ULL;
    if ((int64_t)(deadline_ms - now_ms) <= 0) return false;

    uint64_t delta_ms = deadline_ms - now_ms;

    // Convert ms -> ticks carefully. Avoid portMAX_DELAY(“forever”) semantics.
    const TickType_t ticks_per_sec = configTICK_RATE_HZ;
    const uint64_t   max_ms_per_call =
        ((uint64_t)(portMAX_DELAY - 1) * 1000ULL) / ticks_per_sec;

    while (delta_ms) {
        const uint64_t chunk_ms = (delta_ms > max_ms_per_call) ? max_ms_per_call : delta_ms;
        TickType_t to_wait = pdMS_TO_TICKS(chunk_ms);
        if (to_wait == 0) to_wait = 1;  // round-up for tiny positive deltas
        vTaskDelay(to_wait);
        delta_ms -= chunk_ms;
    }
    return true;
}

bool timing_delay_until_tick(TickType_t abs_tick)
{
    TickType_t now = xTaskGetTickCount();
    // Unsigned subtraction handles wrap-around correctly
    TickType_t to_wait = abs_tick - now;
    if ((int32_t)to_wait <= 0) return false;
    vTaskDelay(to_wait);
    return true;
}

TickType_t timing_ms_to_abs_tick(uint64_t abs_ms)
{
    // ticks(now) + ticks(abs_ms - now_ms)
    const uint64_t now_ms = now_ms_from_us(timing_now_us());
    const uint64_t delta_ms = (abs_ms > now_ms) ? (abs_ms - now_ms) : 0ULL;
    return xTaskGetTickCount() + (TickType_t)pdMS_TO_TICKS(delta_ms);
}

bool timing_sleep_periodic_us(uint64_t *next_t, uint64_t period_us)
{
    if (*next_t == 0) {
        *next_t = timing_now_us() + period_us;
    }

    bool slept = timing_sleep_until_abs_us(*next_t);

    // Catch up if we overran to avoid drift.
    uint64_t now = timing_now_us();
    do {
        *next_t += period_us;
    } while ((int64_t)(*next_t - now) <= 0);

    return slept;
}
