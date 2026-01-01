# PIR Motion Sensor - Configuration & Customization Guide

## Quick Start Configuration

### Basic Setup (No Changes Required)
The code works out-of-the-box with these defaults:
- **PIR Sensor**: GPIO 8 (input)
- **Light Control**: GPIO 8 (output)  
- **Timeout**: 5 minutes (300,000 ms)
- **Debounce**: 100 ms

### Changing GPIO Pins

If you need to use different pins, modify in `app_main.cpp`:

```cpp
pir_light_config_t pir_config = {
    .pir_gpio = GPIO_NUM_8,      // Change this for PIR input
    .light_gpio = GPIO_NUM_9,    // Change this for light output
    .inactivity_timeout_ms = 5 * 60 * 1000,
    .debounce_ms = 100,
};
```

**Available GPIO Pins on ESP32-C3:**
- GPIO 0-7: Available
- GPIO 8-10: Available (used as default)
- GPIO 20-21: JTAG (avoid if using debugger)

### Adjusting Timeout Duration

To change the 5-minute timeout:

```cpp
// For 3 minutes:
.inactivity_timeout_ms = 3 * 60 * 1000

// For 10 minutes:
.inactivity_timeout_ms = 10 * 60 * 1000

// For 30 seconds (testing):
.inactivity_timeout_ms = 30 * 1000
```

### Adjusting Debounce

If your PIR sensor is too sensitive or not sensitive enough:

```cpp
// More sensitive (less debounce):
.debounce_ms = 50

// Less sensitive (more debounce):
.debounce_ms = 200
```

## Advanced Configuration

### Separate Input/Output Pins

If using separate GPIO pins for PIR input and light output:

**Hardware:**
```
GPIO 8  → PIR Sensor Output
GPIO 9  → Light/Relay Control (via transistor)
```

**Code:**
```cpp
pir_light_config_t pir_config = {
    .pir_gpio = GPIO_NUM_8,      // PIR sensor
    .light_gpio = GPIO_NUM_9,    // Light output (transistor/relay)
    .inactivity_timeout_ms = 5 * 60 * 1000,
    .debounce_ms = 100,
};
```

### PWM Light Control (Brightness)

To add brightness control, you'll need to modify `pir_light_control.cpp`:

```cpp
// Add PWM configuration
#include "driver/ledc.h"

// In pir_light_init():
ledc_timer_config_t timer_conf = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 1000,
    .clk_cfg = LEDC_AUTO_CLK,
};
ledc_timer_config(&timer_conf);

// Configure PWM channel
ledc_channel_config_t pwm_conf = {
    .gpio_num = g_config.light_gpio,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_0,
    .duty = 255,  // Full brightness
    .hpoint = 0,
};
ledc_channel_config(&pwm_conf);
```

### Dual Timer Mode

For different timeouts during day/night:

```cpp
// Add second timer configuration
static TimerHandle_t g_inactivity_timer_day = NULL;
static TimerHandle_t g_inactivity_timer_night = NULL;

// Use RTC to determine mode
esp_get_ds_time_t now;
esp_get_dstimeofday(&now);

// Adjust timeout based on time
if (now.tm_hour >= 18 || now.tm_hour < 6) {
    // Night mode: shorter timeout (3 minutes)
    timeout_ms = 3 * 60 * 1000;
} else {
    // Day mode: longer timeout (10 minutes)
    timeout_ms = 10 * 60 * 1000;
}
```

## Sensor Selection & Tuning

### PIR Sensor Types

**Common PIR Modules:**
1. **HC-SR501** (Recommended)
   - Range: 5-7 meters
   - Delay adjustment: 5-300 seconds
   - Sensitivity adjustment: Yes
   
2. **AM312**
   - Range: 3 meters
   - Compact size
   - No adjustments
   
3. **RCWL-0516**
   - Microwave-based (different principle)
   - Better for through-walls
   - Shorter range

### Tuning for Your Space

**For high false positive rate:**
- Reduce debounce: 50ms → 100ms → 150ms
- Use microwave sensor (RCWL-0516)
- Increase inactivity timeout to verify it's working

**For low detection rate:**
- Increase debounce: 100ms → 50ms
- Check sensor's own sensitivity pot
- Ensure proper sensor orientation
- Check power supply stability

## Power Optimization

### Sleep Modes

For battery operation, add deep sleep:

```cpp
// In pir_light_control.cpp, add:

// PIR wakeup trigger
esp_sleep_enable_ext0_wakeup(g_config.pir_gpio, 1);  // Wake on HIGH

// Sleep after light turns off
if (!g_light_on) {
    esp_deep_sleep_start();
}
```

### Power Saving Settings

```cpp
// Increase polling interval to save power
vTaskDelay(pdMS_TO_TICKS(100));  // Was 50ms, now 100ms

// Disable Wi-Fi when battery-powered
// (Matter over Thread alternative)
#ifdef CONFIG_ENABLE_THREAD
// Thread uses less power than Wi-Fi
#endif
```

## Testing & Validation

### Test Cases

1. **Basic Motion Detection**
   ```
   Expected: Motion → Light ON instantly
   Command: Wave hand in front of sensor
   ```

2. **Timer Reset**
   ```
   Expected: Each motion resets the timer
   Command: Move every 3 minutes for 10+ minutes
   Expected: Light stays ON throughout
   ```

3. **Timeout**
   ```
   Expected: Light OFF after 5 minutes no motion
   Command: Stop moving for 5+ minutes
   Expected: Light turns OFF automatically
   ```

4. **Debounce**
   ```
   Expected: No false positives from noise
   Command: Use multimeter to check GPIO for noise
   Check: Debounce prevents false triggers
   ```

### Debugging Output

Monitor serial output:
```bash
idf.py monitor
```

Expected pattern:
```
I (xxx) pir_light: Motion detected, timer reset
I (xxx) pir_light: Light turned ON
I (xxx) pir_light: Inactivity timeout: turning light OFF
I (xxx) pir_light: Light turned OFF
```

## Common Issues & Solutions

| Issue | Cause | Solution |
|-------|-------|----------|
| Light won't turn on | Wrong GPIO | Check GPIO number in config |
| Light stays on permanently | Timer not firing | Check inactivity_timeout_ms |
| Light flickers | Debounce too low | Increase debounce_ms to 150 |
| Slow response | Polling too slow | Reduce delay in polling task |
| No Matter control | Endpoint mismatch | Verify light_endpoint_id |

## Production Checklist

- [ ] Test with your specific PIR sensor
- [ ] Verify all GPIO connections
- [ ] Test Matter commissioning
- [ ] Monitor power consumption
- [ ] Test in actual room conditions
- [ ] Verify timeout works correctly
- [ ] Check Matter app integration
- [ ] Test factory reset procedure
- [ ] Document your specific settings

## References

- [ESP32-C3 GPIO Pins](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3_devkitc-02.html)
- [FreeRTOS Timer API](https://www.freertos.org/RTOS_Timer_Service.html)
- [Matter Specification](https://csa-iot.org/csa_iot_wp_matter_specification_v1_1.pdf)
- [ESP-Matter Documentation](https://github.com/espressif/esp-matter)

---

**Need Help?**
1. Check logs: `idf.py monitor` to see detailed output
2. Verify connections with multimeter
3. Test PIR sensor independently before integration
4. Increase debug logging in pir_light_control.cpp
