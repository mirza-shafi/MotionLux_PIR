
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include "esp_check.h"
#include "pir_light_control.h"

static const char *TAG = "pir_light";

// Global state
static bool g_light_on = false;
static TimerHandle_t g_inactivity_timer = NULL;
static pir_light_config_t g_config = {};

// Timer callback to turn off light after inactivity
static void inactivity_timer_callback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "Inactivity timeout: turning light OFF");
    pir_light_set(false);
}

// PIR polling task
static void pir_polling_task(void *pvParameters) {
    uint32_t last_valid_trigger = 0;
    
    while (true) {
        int pir_state = gpio_get_level(g_config.pir_gpio);
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // Debounce check
        if (pir_state && (now - last_valid_trigger > g_config.debounce_ms)) {
            last_valid_trigger = now;
            
            // Turn on light if not already on
            if (!g_light_on) {
                pir_light_set(true);
            }

            // Reset inactivity timer
            if (g_inactivity_timer != NULL) {
                xTimerReset(g_inactivity_timer, 0);
            }
            
            ESP_LOGI(TAG, "Motion detected, timer reset");
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
        ESP_LOGI(TAG, "Light turned ON");
        // Start inactivity timer
        if (g_inactivity_timer != NULL) {
            xTimerStart(g_inactivity_timer, 0);
        }
    } else {
        ESP_LOGI(TAG, "Light turned OFF");
        // Stop inactivity timer
        if (g_inactivity_timer != NULL) {
            xTimerStop(g_inactivity_timer, 0);
        }
    }

    return ESP_OK;
}
