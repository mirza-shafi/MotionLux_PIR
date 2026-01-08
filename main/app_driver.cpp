/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <esp_matter.h>
#include <app_priv.h>
#include <common_macros.h>

#include <driver/gpio.h>
#include <iot_button.h>
#include <button_gpio.h>

#include "pir_light_control.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;

#define GPIO_LED 8
#define GPIO_BUTTON 9

/* Do any conversions/remapping for the actual value here */
static esp_err_t app_driver_light_set_power(app_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    ESP_LOGI(TAG, "Matter app: Changing light power to %d", val->val.b);
    
    // Set the actual light via PIR control system
    // PIR auto control is always active, app provides manual override
    pir_light_set(val->val.b);
    
    return ESP_OK;
}

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

    attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.b = !val.val.b;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}

static void app_driver_button_factory_reset_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Factory reset triggered via button hold");
    esp_matter::factory_reset();
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == light_endpoint_id) {
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                err = app_driver_light_set_power(driver_handle, val);
            }
        }
    }
    return err;
}

esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    void *priv_data = endpoint::get_priv_data(endpoint_id);
    app_driver_handle_t handle = (app_driver_handle_t)priv_data;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    /* Setting power */
    attribute_t *attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_power(handle, &val);

    return err;
}

app_driver_handle_t app_driver_light_init()
{
    // PIR control system will initialize the light GPIO
    // No separate initialization needed here
    ESP_LOGI(TAG, "Light driver init (GPIO managed by PIR control)");
    
    return (app_driver_handle_t)NULL; // No handle needed for simple GPIO
}

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_config_t btn_cfg = {
        .long_press_time = 5000,
        .short_press_time = 100,
    };

    button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = (int32_t)GPIO_BUTTON,
        .active_level = 0,
    };
    
    button_handle_t handle = NULL;
    esp_err_t err = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &handle);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button device");
        return NULL;
    }

    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, NULL, app_driver_button_toggle_cb, NULL);
    iot_button_register_cb(handle, BUTTON_LONG_PRESS_START, NULL, app_driver_button_factory_reset_cb, NULL);
    
    return (app_driver_handle_t)handle;
}
