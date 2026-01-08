
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include "esp_check.h"
#include "pir_light_control.h"

static const char *TAG = "pir_light";

// Global state
static bool g_light_on = false;
static bool g_auto_control_enabled = false;  // Disabled by default, enabled when light ON
static TimerHandle_t g_inactivity_timer = NULL;
static pir_light_config_t g_config = {};

// Timer callback to turn off light after inactivity
static void inactivity_timer_callback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "[TIMER CALLBACK] Timer expired! g_auto_control_enabled=%d, g_light_on=%d", 
             g_auto_control_enabled, g_light_on);
    
    // Auto turn off after timeout (only if auto control is enabled)
    if (g_auto_control_enabled) {
        ESP_LOGI(TAG, "No motion detected for %ld seconds: turning light OFF automatically", g_config.inactivity_timeout_ms / 1000);
        pir_light_set(false);
    } else {
        ESP_LOGW(TAG, "[TIMER CALLBACK] Auto control is disabled, not turning off light");
    }
}

// PIR polling task
static void pir_polling_task(void *pvParameters) {
    uint32_t last_valid_trigger = 0;
    uint32_t last_debug_print = 0;
    
    while (true) {
        int pir_state = gpio_get_level(g_config.pir_gpio);
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        // Only process PIR if auto control is enabled AND light is ON
        if (g_auto_control_enabled && g_light_on) {

            // Debug: Print GPIO state every 5 seconds
            if (now - last_debug_print > 5000) {
                ESP_LOGI(TAG, "DEBUG: GPIO%d state = %d (0=LOW/no motion, 1=HIGH/motion)", 
                         g_config.pir_gpio, pir_state);
                last_debug_print = now;
            }

            // Debounce check
            // INVERTED LOGIC: Check for LOW (0) instead of HIGH (1) if your PIR is inverted
            // Change "if (pir_state &&" to "if (!pir_state &&" for inverted PIR
            if (pir_state && (now - last_valid_trigger > g_config.debounce_ms)) {
                last_valid_trigger = now;

                // Reset inactivity timer on motion detection (DON'T turn light ON)
                if (g_inactivity_timer != NULL) {
                    BaseType_t timer_reset = xTimerReset(g_inactivity_timer, 0);
                    if (timer_reset == pdPASS) {
                        ESP_LOGI(TAG, "Motion detected: timer RESET (timeout=%ldms)", g_config.inactivity_timeout_ms);
                    } else {
                        ESP_LOGE(TAG, "Motion detected but FAILED to reset timer!");
                    }
                } else {
                    ESP_LOGE(TAG, "Motion detected but timer is NULL!");
                }
            }
        } else {
            // Log why PIR is not monitoring
            if (now - last_debug_print > 5000) {
                if (!g_auto_control_enabled) {
                    ESP_LOGW(TAG, "PIR not monitoring: auto_control_enabled=false (turn light ON via app)");
                }
                if (!g_light_on) {
                    ESP_LOGW(TAG, "PIR not monitoring: light is OFF");
                }
                last_debug_print = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Poll every 50ms
    }
}

// Public API: Initialize PIR light control
esp_err_t pir_light_init(const pir_light_config_t *config) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    g_config = *config;


    // Configure PIR as input
    gpio_config_t pir_conf = {};
    pir_conf.intr_type = GPIO_INTR_DISABLE;
    pir_conf.mode = GPIO_MODE_INPUT;
    pir_conf.pin_bit_mask = (1ULL << g_config.pir_gpio);
    pir_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    pir_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ESP_RETURN_ON_ERROR(gpio_config(&pir_conf), TAG, "Failed to configure PIR GPIO");

    // Configure Light as output
    gpio_config_t light_conf = {};
    light_conf.intr_type = GPIO_INTR_DISABLE;
    light_conf.mode = GPIO_MODE_OUTPUT;
    light_conf.pin_bit_mask = (1ULL << g_config.light_gpio);
    light_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    light_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ESP_RETURN_ON_ERROR(gpio_config(&light_conf), TAG, "Failed to configure Light GPIO");

    // Initialize light to OFF
    gpio_set_level(g_config.light_gpio, 0);
    g_light_on = false;

    // Create inactivity timer
    g_inactivity_timer = xTimerCreate(
        "pir_inactivity",
        pdMS_TO_TICKS(g_config.inactivity_timeout_ms),
        pdFALSE, // One-shot
        NULL,
        inactivity_timer_callback
    );

    if (g_inactivity_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create inactivity timer");
        return ESP_ERR_NO_MEM;
    }

    // Start PIR polling task
    BaseType_t task_result = xTaskCreate(
        pir_polling_task,
        "pir_polling",
        2048,
        NULL,
        5,
        NULL
    );

    if (task_result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create PIR polling task");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "PIR Light Control initialized: PIR=GPIO%d, Light=GPIO%d, Timeout=%ldms",
             g_config.pir_gpio, g_config.light_gpio, g_config.inactivity_timeout_ms);

    return ESP_OK;
}

// Public API: Get light state
bool pir_light_is_on() {
    return g_light_on;
}

// Public API: Set light state
esp_err_t pir_light_set(bool on) {
    if (on == g_light_on) {
        return ESP_OK; // Already in desired state
    }

    // Set GPIO
    gpio_set_level(g_config.light_gpio, on ? 1 : 0);
    g_light_on = on;

    if (on) {
        ESP_LOGI(TAG, "Light turned ON (via app) - PIR monitoring ENABLED");
        // Enable PIR auto control when light turns ON
        g_auto_control_enabled = true;
        // Start inactivity timer
        if (g_inactivity_timer != NULL) {
            BaseType_t timer_started = xTimerStart(g_inactivity_timer, 0);
            if (timer_started == pdPASS) {
                ESP_LOGI(TAG, "[TIMER] Started successfully: Light will auto-OFF after %ld seconds of no motion", g_config.inactivity_timeout_ms / 1000);
            } else {
                ESP_LOGE(TAG, "[TIMER] Failed to start timer!");
            }
        } else {
            ESP_LOGE(TAG, "[TIMER] Timer is NULL, cannot start!");
        }
    } else {
        ESP_LOGI(TAG, "Light turned OFF - PIR monitoring DISABLED");
        // Disable PIR auto control when light turns OFF
        g_auto_control_enabled = false;
        // Stop inactivity timer
        if (g_inactivity_timer != NULL) {
            xTimerStop(g_inactivity_timer, 0);
            ESP_LOGI(TAG, "[TIMER] Stopped");
        }
    }

    return ESP_OK;
}

// Public API: Enable/disable PIR automatic control
void pir_light_enable_auto_control(bool enable) {
    g_auto_control_enabled = enable;
    ESP_LOGI(TAG, "PIR automatic control %s", enable ? "ENABLED" : "DISABLED");
    
    // If disabling auto control, stop the timer
    if (!enable && g_inactivity_timer != NULL) {
        xTimerStop(g_inactivity_timer, 0);
    }
    
    // If enabling auto control and light is ON, start the timer
    if (enable && g_light_on && g_inactivity_timer != NULL) {
        xTimerStart(g_inactivity_timer, 0);
    }
}

// Public API: Check if PIR auto control is enabled
bool pir_light_is_auto_control_enabled() {
    return g_auto_control_enabled;
}
