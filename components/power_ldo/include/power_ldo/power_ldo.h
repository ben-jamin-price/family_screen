#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "esp_err.h"

/**
 * Acquire the on-chip LDO channel used for the MIPI PHY rail
 * and set its target voltage (mirrors your current demo flow).
 *
 * Safe to call once; returns ESP_ERR_INVALID_STATE if already acquired.
 *
 * @param millivolts Target voltage in mV (e.g., 2500)
 * @return ESP_OK on success
 */
esp_err_t power_ldo_enable_mipi_phy_mv(int millivolts);

/**
 * Release the LDO channel if previously acquired.
 * Safe to call even if not acquired (no-op).
 */
void power_ldo_disable_mipi_phy(void);

#ifdef __cplusplus
} // extern "C"
#endif
