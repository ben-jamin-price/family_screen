#include "display_panel/display.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"

#include "board/board.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_dsi.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_heap_caps.h"          // heap_caps_calloc()

#include "esp_lcd_jd9365_10_1.h"    // JD9365 macros + vendor config

typedef struct display_handle_t_ {
    esp_lcd_dsi_bus_handle_t  dsi_bus;
    esp_lcd_panel_io_handle_t dbi_io;
    esp_lcd_panel_handle_t    panel;
    int                       width;
    int                       height;
} display_handle_t_;

static const char *TAG = "display_panel";

/* Same throttling semantics as the working demo. */
static esp_err_t draw_bitmap_throttled(esp_lcd_panel_handle_t panel,
                                       int x0, int y0, int x1, int y1,
                                       const void *buf)
{
    vTaskDelay(3);
    for (int tries = 0; tries < 20; ++tries) {
        esp_err_t err = esp_lcd_panel_draw_bitmap(panel, x0, y0, x1, y1, buf);
        if (err == ESP_OK) return ESP_OK;
        if (err == ESP_ERR_INVALID_STATE) { vTaskDelay(1); continue; }
        return err;
    }
    return ESP_ERR_TIMEOUT;
}

esp_err_t display_init(display_handle_t *out)
{
    ESP_RETURN_ON_FALSE(out, ESP_ERR_INVALID_ARG, TAG, "null out");

    int H = 0, V = 0;
    board_panel_resolution(&H, &V);
    if (H <= 0 || V <= 0) { H = 800; V = 1280; }

    // --- DSI bus (JD9365 2-lane macro) ---
    esp_lcd_dsi_bus_handle_t dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t bus_cfg  = JD9365_PANEL_BUS_DSI_2CH_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_dsi_bus(&bus_cfg, &dsi_bus), TAG, "new dsi bus failed");

    // --- DBI IO (macro config) ---
    esp_lcd_panel_io_handle_t dbi_io = NULL;
    esp_lcd_dbi_io_config_t dbi_cfg  = JD9365_PANEL_IO_DBI_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_dbi(dsi_bus, &dbi_cfg, &dbi_io), TAG, "new dbi io failed");

    // --- DPI panel config (same fields/values as your demo) ---
    const esp_lcd_dpi_panel_config_t dpi_cfg = {
        .dpi_clk_src         = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz  = 40,
        .virtual_channel     = 0,
        .pixel_format        = LCD_COLOR_PIXEL_FORMAT_RGB565,
        .num_fbs             = 2,
        .video_timing = {
            .h_size             = H,
            .v_size             = V,
            .hsync_back_porch   = 20,
            .hsync_pulse_width  = 20,
            .hsync_front_porch  = 40,
            .vsync_back_porch   = 10,
            .vsync_pulse_width  = 4,
            .vsync_front_porch  = 30,
        },
        .flags = {
            .use_dma2d = true,
        },
    };

    // --- Vendor config (MIPI interface enabled) ---
    jd9365_vendor_config_t vcfg = {
        .init_cmds       = NULL,
        .init_cmds_size  = 0,
        .mipi_config = {
            .dsi_bus    = dsi_bus,
            .dpi_config = &dpi_cfg,
            .lane_num   = (uint8_t)board_panel_lane_count(),
        },
        .flags = {
            .use_mipi_interface = 1,
            .mirror_by_cmd      = 0,
        },
    };

    // --- Create panel, reset, init, turn on ---
    esp_lcd_panel_handle_t panel = NULL;
    const esp_lcd_panel_dev_config_t pdev_cfg = {
        .reset_gpio_num   = -1,
        .rgb_ele_order    = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel   = 16,
        .vendor_config    = &vcfg,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_jd9365(dbi_io, &pdev_cfg, &panel), TAG, "new jd9365 failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel), TAG, "panel reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel), TAG, "panel init failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel, true), TAG, "panel on failed");

    // --- Fill handle ---
    display_handle_t_ *h = (display_handle_t_ *)heap_caps_calloc(1, sizeof(*h), MALLOC_CAP_DEFAULT);
    ESP_RETURN_ON_FALSE(h, ESP_ERR_NO_MEM, TAG, "alloc handle failed");
    h->dsi_bus = dsi_bus;
    h->dbi_io  = dbi_io;
    h->panel   = panel;
    h->width   = H;
    h->height  = V;

    *out = (display_handle_t)h;
    const int clk_mhz = dpi_cfg.dpi_clock_freq_mhz;
    const int lanes   = (int)vcfg.mipi_config.lane_num;
    const int fbs     = dpi_cfg.num_fbs;

    ESP_LOGI(TAG, "JD9365 panel ready (%dx%d, RGB565, %dMHz, DMA2D, %d FBs, lanes=%d)",
            H, V, clk_mhz, fbs, lanes);
    return ESP_OK;
}

int display_width(display_handle_t d)
{
    display_handle_t_ *h = (display_handle_t_ *)d;
    return h ? h->width : 0;
}

int display_height(display_handle_t d)
{
    display_handle_t_ *h = (display_handle_t_ *)d;
    return h ? h->height : 0;
}

esp_err_t display_draw_bitmap(display_handle_t d,
                              int x0, int y0, int x1, int y1,
                              const void *buf)
{
    display_handle_t_ *h = (display_handle_t_ *)d;
    if (!h || !buf) return ESP_ERR_INVALID_ARG;
    return draw_bitmap_throttled(h->panel, x0, y0, x1, y1, buf);
}

esp_err_t display_on(display_handle_t d, bool on)
{
    display_handle_t_ *h = (display_handle_t_ *)d;
    if (!h) return ESP_ERR_INVALID_ARG;
    return esp_lcd_panel_disp_on_off(h->panel, on);
}
