#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "driver/i2c_master.h"  // for i2c_port_t

/** One-time board bring-up hook (carrier defaults are fine for now). */
void board_init(void);

/** Fixed panel characteristics for this carrier + module. */
int  board_panel_lane_count(void);            // e.g., 2 DSI lanes
void board_panel_resolution(int *w, int *h);  // e.g., 800 x 1280

/** GT9xx touch wiring (for modern i2c_master API). */
i2c_port_t board_touch_i2c_port(void);        // e.g., I2C_NUM_1
void board_get_touch_pins(int *sda, int *scl, int *rst, int *int_pin);
uint8_t  board_touch_i2c_address(void);       // 0x5D
uint32_t board_touch_safe_scl_hz(void);       // 100 kHz

#ifdef __cplusplus
} // extern "C"
#endif
