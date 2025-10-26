#include "board/board.h"
#include "esp_log.h"

static const char *TAG = "board";

/* Panel traits (Waveshare 10.1" JD9365 via DSI) */
#define PANEL_H_RES        800
#define PANEL_V_RES        1280
#define PANEL_DSI_LANES    2

/* Touch (GT911) wiring on ESP32-P4-Nano carrier */
#define TP_I2C_PORT        I2C_NUM_1
#define TP_SDA_GPIO        18
#define TP_SCL_GPIO        17
#define TP_INT_GPIO        -1   // not wired
#define TP_RST_GPIO        15   // -1 if not wired
#define TP_I2C_ADDR        0x5D
#define TP_SCL_SAFE_HZ     100000u

void board_init(void)
{
    ESP_LOGI(TAG, "board_init: carrier defaults are sufficient");
}

int board_panel_lane_count(void)
{
    return PANEL_DSI_LANES;
}

void board_panel_resolution(int *w, int *h)
{
    if (w) *w = PANEL_H_RES;
    if (h) *h = PANEL_V_RES;
}

i2c_port_t board_touch_i2c_port(void)
{
    return TP_I2C_PORT;
}

void board_get_touch_pins(int *sda, int *scl, int *rst, int *int_pin)
{
    if (sda)     *sda     = TP_SDA_GPIO;
    if (scl)     *scl     = TP_SCL_GPIO;
    if (rst)     *rst     = TP_RST_GPIO;   // -1 if unused
    if (int_pin) *int_pin = TP_INT_GPIO;   // -1 if unused
}

uint8_t board_touch_i2c_address(void)
{
    return TP_I2C_ADDR;
}

uint32_t board_touch_safe_scl_hz(void)
{
    return TP_SCL_SAFE_HZ;
}
