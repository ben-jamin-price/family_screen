#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/** Opaque display handle. */
typedef struct display_handle_t_* display_handle_t;

/**
 * Initialize the JD9365 10.1" DSI panel on ESP32-P4-Nano.
 * Sequence mirrors the known-good demo (DSI bus, DBI IO, DPI panel, 2 FBs, DMA2D).
 *
 * @param[out] out  Handle returned on success (unchanged on failure).
 * @return ESP_OK on success.
 */
esp_err_t display_init(display_handle_t *out);

/** Logical resolution (same as JD9365 timing). */
int display_width(display_handle_t d);
int display_height(display_handle_t d);

/**
 * Throttled draw wrapper around esp_lcd_panel_draw_bitmap().
 * Retries on ESP_ERR_INVALID_STATE (panel busy).
 */
esp_err_t display_draw_bitmap(display_handle_t d,
                              int x0, int y0, int x1, int y1,
                              const void *buf);

/** Turn panel on/off. */
esp_err_t display_on(display_handle_t d, bool on);

#ifdef __cplusplus
} // extern "C"
#endif
