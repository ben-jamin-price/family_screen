#include "touch_gt9xx/touch_gt9xx.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_idf_version.h"      // for ESP_IDF_VERSION* macros

#include "board/board.h"
#include "driver/i2c_master.h"
#include "i2c_bus.h"              // helper wrapper used by your main

static const char *TAG = "touch_gt9xx";

/* GT911 register map (subset) */
#define GT_REG_STATUS      0x814E   // [7]=ready, [3:0]=num points
#define GT_REG_POINT1      0x8150   // P0: xL,xH,yL,yH,id,wL,wH,...
#define GT_REG_DEV_MODE    0x8040   // 0x00 normal
#define GT_REG_PID         0x8140   // 4 bytes
#define GT_REG_FWVER       0x8144   // 2 bytes
#define GT_REG_XY_MAX      0x8048   // X L,H, Y L,H

typedef struct touch_handle_t_ {
    i2c_bus_handle_t        bus_outer;
    i2c_master_bus_handle_t bus_raw;
    i2c_master_dev_handle_t dev;
    uint16_t xmax, ymax;
} touch_handle_t_;

/* ---- low-level helpers on modern i2c_master_* ---- */
static esp_err_t gt_reg_read(i2c_master_dev_handle_t dev, uint16_t reg, void *buf, size_t len)
{
    uint8_t hdr[2] = { (uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF) };
    ESP_RETURN_ON_ERROR(i2c_master_transmit(dev, hdr, 2, 20), TAG, "tx hdr");
    return i2c_master_receive(dev, buf, len, 20);
}

static esp_err_t gt_reg_write(i2c_master_dev_handle_t dev, uint16_t reg, const void *buf, size_t len)
{
    uint8_t tmp[2 + 16];  // increase if writing >16 bytes
    if (len > 16) return ESP_ERR_INVALID_SIZE;
    tmp[0] = (uint8_t)(reg >> 8);
    tmp[1] = (uint8_t)(reg & 0xFF);
    memcpy(&tmp[2], buf, len);
    return i2c_master_transmit(dev, tmp, 2 + len, 20);
}

/* ---- public API ---- */
esp_err_t touch_gt9xx_init(touch_handle_t *out)
{
    ESP_RETURN_ON_FALSE(out, ESP_ERR_INVALID_ARG, TAG, "null out");

    esp_err_t ret = ESP_OK;  // required for ESP_GOTO_ON_ERROR
    const i2c_port_t port = board_touch_i2c_port();
    int sda = 0, scl = 0, rst = -1, intr = -1;
    board_get_touch_pins(&sda, &scl, &rst, &intr);
    const uint8_t  addr = board_touch_i2c_address();
    const uint32_t hz   = board_touch_safe_scl_hz();

    touch_handle_t_ *h = calloc(1, sizeof(*h));
    if (!h) { ret = ESP_ERR_NO_MEM; goto err; }

    // Outer bus (matches your working example)
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = true,
        .scl_pullup_en = true,
        .master.clk_speed = hz,
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        .clk_flags = LP_I2C_SCLK_DEFAULT,
    #endif
    };
    h->bus_outer = i2c_bus_create(port, &conf);
    if (!h->bus_outer) { ESP_LOGE(TAG, "i2c_bus_create failed"); ret = ESP_FAIL; goto err; }

    h->bus_raw = i2c_bus_get_internal_bus_handle(h->bus_outer);
    if (!h->bus_raw) { ESP_LOGE(TAG, "no raw bus"); ret = ESP_FAIL; goto err; }

    // Add GT911 device
    i2c_device_config_t dev_cfg = {
        .device_address = addr,
        .scl_speed_hz   = hz,
    };
    ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(h->bus_raw, &dev_cfg, &h->dev),
                      err, TAG, "add dev");

    // Wake + clear
    uint8_t zero = 0x00;
    ESP_GOTO_ON_ERROR(gt_reg_write(h->dev, GT_REG_DEV_MODE, &zero, 1), err, TAG, "mode");
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_GOTO_ON_ERROR(gt_reg_write(h->dev, GT_REG_STATUS,  &zero, 1), err, TAG, "clr status");

    // Best-effort info (ignore errors)
    char pid[4] = {0}; uint8_t fw[2] = {0}, xy[4] = {0};
    (void)gt_reg_read(h->dev, GT_REG_PID,    pid, sizeof(pid));
    (void)gt_reg_read(h->dev, GT_REG_FWVER,  fw,  sizeof(fw));
    (void)gt_reg_read(h->dev, GT_REG_XY_MAX, xy,  sizeof(xy));
    h->xmax = (uint16_t)((xy[1] << 8) | xy[0]);
    h->ymax = (uint16_t)((xy[3] << 8) | xy[2]);
    uint16_t fwver = (uint16_t)((fw[1] << 8) | fw[0]);
    ESP_LOGI(TAG, "GT9xx PID='%c%c%c%c' FW=%u X_MAX=%u Y_MAX=%u",
             pid[0], pid[1], pid[2], pid[3], fwver, h->xmax, h->ymax);

    *out = (touch_handle_t)h;
    return ESP_OK;

err:
    if (h) {
        if (h->dev)       (void)i2c_master_bus_rm_device(h->dev);
        if (h->bus_outer) (void)i2c_bus_delete(h->bus_outer);
        free(h);
    }
    return ret;
}

bool touch_gt9xx_read_first(touch_handle_t t, uint16_t *x, uint16_t *y)
{
    touch_handle_t_ *h = (touch_handle_t_ *)t;
    if (!h) return false;

    uint8_t status = 0;
    if (gt_reg_read(h->dev, GT_REG_STATUS, &status, 1) != ESP_OK) return false;

    const uint8_t count = status & 0x0F;
    const bool ready    = (status & 0x80) != 0;
    if (!(ready && count > 0)) {
        if (ready) { uint8_t zero = 0; (void)gt_reg_write(h->dev, GT_REG_STATUS, &zero, 1); }
        return false;
    }

    uint8_t p0[8] = {0};
    if (gt_reg_read(h->dev, GT_REG_POINT1, p0, sizeof(p0)) != ESP_OK) return false;

    if (x) *x = (uint16_t)((p0[1] << 8) | p0[0]);
    if (y) *y = (uint16_t)((p0[3] << 8) | p0[2]);

    uint8_t zero = 0;
    (void)gt_reg_write(h->dev, GT_REG_STATUS, &zero, 1);  // ACK/clear
    return true;
}

esp_err_t touch_gt9xx_dump_info(touch_handle_t t, char pid[4], uint16_t *fwver,
                                uint16_t *xmax, uint16_t *ymax)
{
    touch_handle_t_ *h = (touch_handle_t_ *)t;
    ESP_RETURN_ON_FALSE(h, ESP_ERR_INVALID_ARG, TAG, "null handle");

    char lpid[4] = {0}; uint8_t fw[2] = {0}; uint8_t xy[4] = {0};
    ESP_RETURN_ON_ERROR(gt_reg_read(h->dev, GT_REG_PID,   lpid, sizeof(lpid)), TAG, "pid");
    ESP_RETURN_ON_ERROR(gt_reg_read(h->dev, GT_REG_FWVER, fw,   sizeof(fw)),   TAG, "fw");
    ESP_RETURN_ON_ERROR(gt_reg_read(h->dev, GT_REG_XY_MAX,xy,   sizeof(xy)),   TAG, "xy");

    if (pid)   memcpy(pid, lpid, 4);
    if (fwver) *fwver = (uint16_t)((fw[1] << 8) | fw[0]);
    if (xmax)  *xmax  = (uint16_t)((xy[1] << 8) | xy[0]);
    if (ymax)  *ymax  = (uint16_t)((xy[3] << 8) | xy[2]);
    return ESP_OK;
}

void touch_gt9xx_deinit(touch_handle_t t)
{
    touch_handle_t_ *h = (touch_handle_t_ *)t;
    if (!h) return;

    if (h->dev)       (void)i2c_master_bus_rm_device(h->dev);
    if (h->bus_outer) (void)i2c_bus_delete(h->bus_outer);
    memset(h, 0, sizeof(*h));
    free(h);
}
