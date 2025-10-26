// ESP32-P4 Nano + Waveshare 10.1" DSI: bring-up, framebuffer, touch menu, demo fallback.
// Keep app_main linear; push details into board/display/touch modules.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"

#include "board/board.h"
#include "power_ldo/power_ldo.h"
#include "display_panel/display.h"
#include "touch_gt9xx/touch_gt9xx.h"
#include "ui_menu/menu.h"
#include "demos/demos.h"
#include "util/fb.h"

static const char *TAG = "app_main";
static const int kMipiPhyMv = 2500;  // JD9365 PHY rail target (matches demo wiring)

void app_main(void)
{
    // Board: clocks, pins, carrier specifics.
    board_init();

    // MIPI PHY rail: request ~2.5 V. Safe if carrier already powers it.
    // Note: esp_ldo_enable_channel/disable_channel do not exist; see power_ldo.c.
    esp_err_t err = power_ldo_enable_mipi_phy_mv(kMipiPhyMv);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "MIPI LDO setup failed: %s", esp_err_to_name(err));
        // Proceed: panel may already be powered by carrier.
    }

    // Panel: init JD9365 via display module.
    display_handle_t disp = NULL;
    err = display_init(&disp);
    if (err != ESP_OK || !disp) {
        ESP_LOGE(TAG, "Display init failed: %s", esp_err_to_name(err));
        abort();
    }

    const int W = display_width(disp);
    const int H = display_height(disp);
    ESP_LOGI(TAG, "Display online @ %dx%d (RGB565)", W, H);

    // Framebuffer: PSRAM + DMA-capable when available; start from a known frame.
    const size_t npx   = (size_t)W * (size_t)H;
    const size_t bytes = npx * sizeof(uint16_t);
    uint16_t *fb = (uint16_t *)util_malloc_psram_dma(bytes);
    if (!fb) {
        ESP_LOGE(TAG, "Framebuffer alloc failed (%zu bytes)", bytes);
        abort();
    }
    memset(fb, 0x00, bytes);
    (void)display_draw_bitmap(disp, 0, 0, W, H, fb);

    // Touch (GT9xx): menu on success; otherwise fall back to light demos.
    touch_handle_t touch = NULL;
    err = touch_gt9xx_init(&touch);
    if (err == ESP_OK && touch) {
        // Menu owns interaction loop; returns only on error/exit.
        menu_draw(disp, fb);
        menu_loop(disp, fb, touch);
        ESP_LOGW(TAG, "menu_loop returned; switching to demo carousel");
    } else {
        ESP_LOGW(TAG, "Touch init failed (%s); running demo carousel",
                 esp_err_to_name(err));
    }

    // Fallback demo carousel: keeps panel exercised.
    for (;;) {
        demo_color_bars(disp, fb, 2);         vTaskDelay(pdMS_TO_TICKS(600));
        demo_vertical_gradient(disp, fb, 2);  vTaskDelay(pdMS_TO_TICKS(600));
        demo_bounce_seconds(disp, fb, 5);     vTaskDelay(pdMS_TO_TICKS(300));
        demo_checker_sleep_wake(disp, fb, 2); vTaskDelay(pdMS_TO_TICKS(600));
    }

    // (Unreached in normal flow)
    // TODO: graceful shutdown hook to blank panel, free FB, and (if applicable) release rail.
    // touch_gt9xx_deinit(touch);
}
