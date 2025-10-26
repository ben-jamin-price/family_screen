#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/** Opaque touch handle */
typedef struct touch_handle_t_* touch_handle_t;

/**
 * Initialize GT9xx (GT911) on the board’s touch bus.
 * - Creates the I2C bus and device at the board-selected address
 * - Wakes the device, clears status
 * - Logs PID/FW/XY once (best-effort)
 *
 * @param[out] out  Handle on success (unchanged on failure)
 * @return ESP_OK on success
 */
esp_err_t touch_gt9xx_init(touch_handle_t *out);

/**
 * Read the first touch point (P0), if present.
 * On success, acknowledges (clears) the touch status register.
 *
 * @return true if a point was read; false otherwise
 */
bool touch_gt9xx_read_first(touch_handle_t t, uint16_t *x, uint16_t *y);

/**
 * Optional: fetch device info. Any argument may be NULL.
 * pid[4] is copied raw (no terminator).
 */
esp_err_t touch_gt9xx_dump_info(touch_handle_t t, char pid[4],
                                uint16_t *fwver, uint16_t *xmax, uint16_t *ymax);

/** Graceful shutdown: remove device and delete bus. Safe to call once. */
void touch_gt9xx_deinit(touch_handle_t t);

#ifdef __cplusplus
} // extern "C"
#endif
