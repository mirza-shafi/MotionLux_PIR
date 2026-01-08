#pragma once

#include "driver/gpio.h"
#include "freertos/timers.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

// PIR Light Control Configuration
typedef struct {
    gpio_num_t pir_gpio;
    gpio_num_t light_gpio;
    uint32_t inactivity_timeout_ms;
    uint32_t debounce_ms;
} pir_light_config_t;

// Initialize PIR light control system
esp_err_t pir_light_init(const pir_light_config_t *config);

// Get current light state
bool pir_light_is_on();

// Manually set light (can be called from Matter attribute callbacks)
esp_err_t pir_light_set(bool on);

// Enable or disable PIR automatic control
void pir_light_enable_auto_control(bool enable);

// Check if PIR auto control is enabled
bool pir_light_is_auto_control_enabled();

#ifdef __cplusplus
}
#endif
