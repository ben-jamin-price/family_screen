#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/** Monotonic time since boot (µs). */
uint64_t timing_now_us(void);

/**
 * Sleep until an absolute microsecond deadline.
 * Uses RTOS delay for the coarse part, then a short busy-wait to finish near target.
 *
 * @return true if we actually slept; false if deadline already passed.
 */
bool timing_sleep_until_abs_us(uint64_t deadline_us);

/**
 * Sleep until an absolute millisecond deadline using RTOS ticks only.
 * Lowest-CPU option; timing is quantized to 1 tick and may finish slightly late.
 *
 * @return true if we actually delayed; false if deadline already passed.
 */
bool timing_sleep_until_abs_ms(uint64_t deadline_ms);

/**
 * Sleep until an absolute RTOS tick deadline (wrap-safe).
 *
 * @return true if we actually slept; false if deadline already passed.
 */
bool timing_delay_until_tick(TickType_t abs_tick);

/**
 * Convert an absolute millisecond time (same epoch as timing_now_us) to an absolute tick.
 */
TickType_t timing_ms_to_abs_tick(uint64_t abs_ms);

/** Alias of xTaskGetTickCount(). */
static inline TickType_t timing_now_ticks(void) { return xTaskGetTickCount(); }

/**
 * Periodic helper: sleep until *next_t, then advance by period.
 * If *next_t == 0, it initializes to now + period_us.
 * After waking (or if late), it increments *next_t until it is strictly > now
 * to avoid long-term drift after overruns.
 */
bool timing_sleep_periodic_us(uint64_t *next_t, uint64_t period_us);

#ifdef __cplusplus
}
#endif
