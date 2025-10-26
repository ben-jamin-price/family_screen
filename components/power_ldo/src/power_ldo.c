#include "power_ldo/power_ldo.h"

#include "esp_check.h"
#include "esp_log.h"
#include "esp_ldo_regulator.h"  // IDF LDO API

static const char *TAG = "power_ldo";

/* Handle for the acquired channel so we can release later. */
static esp_ldo_channel_handle_t s_ldo = NULL;

/* Channel used by the MIPI PHY rail in your demo. */
#ifndef POWER_LDO_MIPI_PHY_CHAN
#define POWER_LDO_MIPI_PHY_CHAN  3
#endif

esp_err_t power_ldo_enable_mipi_phy_mv(int millivolts)
{
    ESP_RETURN_ON_FALSE(millivolts > 0, ESP_ERR_INVALID_ARG, TAG, "invalid mV");
    if (s_ldo) {
        ESP_LOGW(TAG, "LDO already acquired");
        return ESP_ERR_INVALID_STATE;
    }

    /* Matches your demo:
       - acquire channel 3
       - set target voltage before any optional enable
       (enable call remains intentionally commented out) */
    esp_ldo_channel_config_t cfg = {
        .chan_id    = POWER_LDO_MIPI_PHY_CHAN,
        .voltage_mv = millivolts,
    };

    ESP_RETURN_ON_ERROR(esp_ldo_acquire_channel(&cfg, &s_ldo), TAG, "acquire failed");
    // ESP_RETURN_ON_ERROR(esp_ldo_enable_channel(s_ldo),          TAG, "enable failed");

    ESP_LOGI(TAG, "MIPI PHY LDO ch%d acquired @ %d mV", POWER_LDO_MIPI_PHY_CHAN, millivolts);
    return ESP_OK;
}

void power_ldo_disable_mipi_phy(void)
{
    if (!s_ldo) return;

    // esp_err_t err1 = esp_ldo_disable_channel(s_ldo);
    esp_err_t err2 = esp_ldo_release_channel(s_ldo);
    s_ldo = NULL;

    // if (err1 != ESP_OK) ESP_LOGW(TAG, "disable returned %d", (int)err1);
    if (err2 != ESP_OK) ESP_LOGW(TAG, "release returned %d", (int)err2);

    ESP_LOGI(TAG, "MIPI PHY LDO released");
}
