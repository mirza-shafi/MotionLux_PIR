# PIR Motion Sensor - Architecture & Flow Diagrams

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32-C3 SuperMini                       │
│  ┌──────────────────────────────────────────────────────┐   │
│  │                   app_main()                         │   │
│  │  • Initialize Matter                                │   │
│  │  • Initialize PIR Light Control                     │   │
│  │  • Keep task alive                                  │   │
│  └──────────────────────────────────────────────────────┘   │
│           ↓                                ↓                 │
│  ┌──────────────────────┐    ┌─────────────────────────┐    │
│  │ pir_light_control    │    │   Matter Stack          │    │
│  │  Module              │    │  (CHIP/Thread/Wi-Fi)    │    │
│  │                      │    │                         │    │
│  │ • PIR Polling Task   │←──→│ • OnOff Cluster         │    │
│  │ • Inactivity Timer   │    │ • Light Endpoint        │    │
│  │ • GPIO Control       │    │ • Commissioning         │    │
│  │ • State Management   │    │                         │    │
│  └──────────────────────┘    └─────────────────────────┘    │
│           ↓                           ↓                      │
└───────────┼───────────────────────────┼──────────────────────┘
            │                           │
            │                           │
    ┌───────▼──────────┐       ┌────────▼────────────┐
    │  GPIO Hardware   │       │  Matter Network     │
    │  • GPIO 8 Input  │       │  • Wi-Fi / Thread   │
    │  • GPIO 8 Output │       │  • BLE (Commis.)    │
    └────────┬─────────┘       └─────────────────────┘
             │
    ┌────────┴─────────────────┐
    │                          │
┌───▼────┐            ┌────────▼──┐
│   PIR  │            │  Light    │
│ Sensor │            │ Relay/LED │
│        │            │  (GPIO 8) │
└────────┘            └───────────┘
```

## State Machine Diagram

```
                           ┌──────────────────────┐
                           │   Initial State      │
                           │   Light: OFF         │
                           └──────────┬───────────┘
                                      │
                    ┌─────────────────┴──────────────────┐
                    │ PIR Detects Motion                 │
                    │ (Debounce passed)                  │
                    ↓                                     ↓
            ┌───────────────────┐              ┌──────────────────┐
            │  Light ON State   │              │  Timer not yet   │
            │                   │              │  started         │
            │ • GPIO = 1        │              │                  │
            │ • Matter sync'd   │              └────────┬─────────┘
            │ • Timer started   │                       │
            └────────┬──────────┘                       │
                     │                                   │
        ┌────────────┴───────────────┐                 │
        │                            │                 │
   Motion    │ No Motion &          │ Still for       │
Continues   │ Timer Running        5 minutes         │
        │                            │                 │
        ↓                            ↓                 ↓
    ┌────────────────────────────────────────────┐
    │          Inactivity Timeout                │
    │          Timer Callback Fires              │
    │                                            │
    │  • GPIO = 0                                │
    │  • Matter sync'd                           │
    │  • Timer stopped                           │
    └────────────┬───────────────────────────────┘
                 │
                 │
            ┌────▼────────────┐
            │  Light OFF      │
            │  Ready for next │
            │  motion event   │
            └─────────────────┘
```

## Timer Reset Mechanism

```
Motion Event Timeline:
─────────────────────────────────────────────────────────────

Time (seconds):    0      30      60      90      120     150     160
                   │       │       │       │       │       │       │
Motion Events:     M       M       M       M       -       -       M
                   │       │       │       │       │       │       │
Timer Status:      ↻       ↻       ↻       ↻       ⏱       ⏱       ↻
                   │       │       │       │       │       │       │
Light State:       ON      ON      ON      ON      ON      ON      ON
                   │       │       │       │       │       │       │
Reset Countdown:   300     300     300     300     270     240     300
                                                    ↑ counts down ↑ reset
───────────────────────────────────────────────────────────────
                                         ↓ 240 seconds remaining
                                    (4 minutes left until OFF)


Timeout Event:
─────────────────────────────────────────────────────────────

Time (seconds):    0       ...     298     299     300
                   │       │       │       │       │
Motion Events:     M       (none)  -       -       -
                   │       │       │       │       │
Timer Status:      ↻       ⏱       ⏱       ⏱       ✗ FIRED
                   │       │       │       │       │
Light State:       ON      ON      ON      ON      OFF
                   │       │       │       │       │
Reset Countdown:   300     ...     2       1       0 (Light OFF)
─────────────────────────────────────────────────────────────
                                         Timeout → Light OFF
```

## GPIO Pin Configuration

```
ESP32-C3 Pinout (PIR/Light setup):

     ┌─────────────────────────┐
     │   ESP32-C3 SuperMini    │
     │                         │
     │  GND ──────────┬────── GND
     │  5V ───────────│────── 5V (if needed)
     │  3.3V ─────────│────── 3.3V (PIR VCC)
     │  GPIO8 ────────┼────── PIR OUT (Input)
     │                │
     │  GPIO8 ────────┼────── Transistor/Relay Gate
     │                │       (Light Control Output)
     │  GND ──────────┴────── GND (common)
     │                         │
     └─────────────────────────┘
                        ↓
            ┌─────────────────────┐
            │  Light Circuit      │
            │  ────────────────   │
            │  GPIO8 ─┤NPN Trans. │
            │         └──→ Relay  │
            │            └─→ LED  │
            │               (24V) │
            └─────────────────────┘

Note: Use appropriate transistor/relay for your light!
      (MOSFET, NPN transistor, or relay module)
```

## Software Module Interaction

```
┌─────────────────────────────────────────────────────┐
│              app_main.cpp                           │
│  • Initializes Matter                              │
│  • Sets up light endpoint                          │
│  • Calls pir_light_init()                          │
│  • Main event loop                                 │
└────────────────┬────────────────────────────────────┘
                 │
                 │ calls
                 ↓
┌─────────────────────────────────────────────────────┐
│         pir_light_control.h (Public API)            │
│  • pir_light_init()                                │
│  • pir_light_set()                                 │
│  • pir_light_is_on()                               │
└────────────────┬────────────────────────────────────┘
                 │
                 │ implements
                 ↓
┌─────────────────────────────────────────────────────┐
│         pir_light_control.cpp (Internal)            │
│  • inactivity_timer_callback()                     │
│  • pir_polling_task()                              │
│  • g_light_on (state)                              │
│  • g_inactivity_timer (handle)                     │
│  • g_matter_endpoint_id                            │
└────────────────┬────────────────────────────────────┘
                 │
     ┌───────────┴───────────┬────────────────┐
     │                       │                │
     ↓                       ↓                ↓
┌─────────┐         ┌──────────────┐  ┌──────────────┐
│ gpio.h  │         │ timers.h     │  │ esp_matter.h │
│ (GPIO)  │         │ (FreeRTOS)   │  │ (Matter)     │
└─────────┘         └──────────────┘  └──────────────┘
```

## Debounce Logic

```
PIR Sensor Output (Raw - with noise):
─────────────────────────────────────────────

         ┌─┐                      ┌─┐
    ┌────┘ └──┬──────────────┬───┘ └─────────
    │ noise   │              │ noise
   HIGH      LOW           HIGH


Debounce Filter (100ms):
─────────────────────────────────────────────

Read every 50ms:
[HIGH] [HIGH] [LOW] [HIGH] [HIGH] [HIGH] [HIGH] [HIGH]
  ↓      ↓     ↓     ↓      ↓      ↓      ↓      ↓
  0      50    100   150    200    250    300   350ms
  
  ✗Skip  ✗Skip ✗Ignore ✓Found (stable for 100ms!)


Filtering Rule:
─────────────────────────────────────────────
Only trigger if:
  • Current read: HIGH
  • Time since last trigger: > 100ms
  
This filters noise spikes < 100ms duration


Result (Debounced):
─────────────────────────────────────────────
         ┌──────────────────────┐
    ─────┘                      └──────────
    
    (Clean signal, no false triggers)
```

## Timing Sequence Diagram

```
Time (t)  Component    Action              State
─────────────────────────────────────────────────────
t=0       PIR          Motion detected      ───┐
          app_main()   Call pir_light_init() │
          GPIO         GPIO8 = 0 (OFF)      │
                                            │
t=100ms   pir_task()   Read PIR (debounce)  │
          pir_task()   Motion confirmed     ─┼─ Motion
                                            │ phase
t=110ms   pir_task()   Call pir_light_set() │
          GPIO         GPIO8 = 1 (ON)      ─┼─
          Matter       Update OnOff=TRUE   │
          Timer        Start 5min timer    ─┘
          
t=100s    (no motion)  Timer counting...   ──── Inactivity
          Timer        Time left: 200s     phase
          GPIO         GPIO8 = 1 (still ON)

t=250s    PIR          Motion again!       ───┐
          pir_task()   Debounce check OK   ─┼─ Motion
          Timer        xTimerReset()       │ reset
          Timer        Time left: 300s     ─┘
          
t=550s    (no motion)  Timer firing        ───┐
          Timer        Timeout callback    │
          GPIO         GPIO8 = 0 (OFF)    ─┼─ Timeout
          Matter       Update OnOff=FALSE  │ event
          LED          Physical light OFF  ─┘
```

## Data Flow During Motion Event

```
Motion Detection Flow:
─────────────────────

1. PIR Sensor detects movement
   │
   ↓
2. GPIO8 = HIGH (100+ milliseconds stable)
   │
   ↓
3. pir_polling_task reads GPIO (every 50ms)
   │
   ├─→ pir_state = HIGH
   │
   ├─→ Check: now - last_trigger > 100ms?
   │
   ↓ YES
4. pir_light_set(true) called
   │
   ├─→ gpio_set_level(GPIO8, 1) → Light ON
   │
   ├─→ g_light_on = true
   │
   ├─→ esp_matter_attr_val_t val = true
   │
   ├─→ attribute::set() → Matter updated
   │
   ├─→ xTimerReset() → Timer reset to 5 minutes
   │
   └─→ ESP_LOGI() → Serial log output
   
5. Light is now ON, timer is running
   │
   ↓
6. If motion continues: Timer keeps resetting
   
7. If no motion for 5 minutes:
   │
   ├─→ inactivity_timer_callback() fires
   │
   ├─→ pir_light_set(false)
   │
   ├─→ gpio_set_level(GPIO8, 0) → Light OFF
   │
   └─→ Matter updated
```

## Error Handling Flow

```
pir_light_init() Called:
───────────────────────

Config Check
   ├─→ NULL? Return ESP_ERR_INVALID_ARG
   │
GPIO Config (PIR)
   ├─→ Fail? Return ESP_ERR_INVALID_STATE
   │
GPIO Config (Light)
   ├─→ Fail? Return ESP_ERR_INVALID_STATE
   │
Timer Creation
   ├─→ NULL? Return ESP_ERR_NO_MEM
   │
Task Creation
   ├─→ Fail? Return ESP_ERR_NO_MEM
   │
All OK?
   └─→ Return ESP_OK, log initialization


app_main() Usage:
────────────────

err = pir_light_init(&config, endpoint_id)
   │
   ├─→ Check: err == ESP_OK?
   │
   ├─→ NO:  ABORT_APP_ON_FAILURE()
   │        Log error
   │        Halt
   │
   └─→ YES: Continue normally
```

---

These diagrams provide a complete visual understanding of:
- System architecture
- State transitions
- Timing and synchronization
- GPIO configuration
- Data flows
- Error handling

Refer to these when debugging or customizing the system!
