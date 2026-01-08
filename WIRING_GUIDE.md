# MotionLux PIR - Wiring Guide

## Hardware Components Required

### Main Components
1. **ESP32-C3 Supermini** - Main controller board
2. **PIR Motion Sensor** (HC-SR501 or similar)
3. **Relay Module** (5V, 1-channel) or **MOSFET/Transistor circuit**
4. **Light/LED** (appropriate for your relay rating)
5. **Connecting Wires**
6. **Power Supply** (5V 2A recommended)
7. **Optional: Push Button** for factory reset

### Component Specifications

#### PIR Sensor (HC-SR501)
- **Operating Voltage**: 5V DC (some models: 3.3V - 5V)
- **Output**: Digital HIGH (3.3V) when motion detected
- **Detection Range**: 3-7 meters (adjustable)
- **Detection Angle**: 110° cone angle
- **Delay Time**: Adjustable (0.5s - 200s) - not used in code
- **Trigger Mode**: Repeatable trigger recommended

#### Relay Module
- **Operating Voltage**: 5V DC
- **Control Signal**: 3.3V compatible
- **Contact Rating**: Match to your light (e.g., 10A @ 250V AC)
- **Isolation**: Optical isolation recommended
- **Type**: Active LOW or Active HIGH (adjust code if needed)

#### ESP32-C3 Supermini
- **Operating Voltage**: 3.3V (USB 5V input)
- **GPIO Voltage**: 3.3V
- **Max Current per GPIO**: 40mA (use relay, not direct)

## Wiring Diagram (ASCII)

```
                     +5V Power Supply
                          │
                ┌─────────┴──────────┐
                │                    │
                │                    │
        ┌───────▼─────────┐   ┌─────▼──────┐
        │   ESP32-C3      │   │ PIR Sensor │
        │   Supermini     │   │ (HC-SR501) │
        │                 │   │            │
        │  5V/USB ────────┼───│ VCC        │
        │  GND ───────────┼───│ GND        │
        │                 │   │            │
        │  GPIO10 ────────┼───│ OUT        │
        │                 │   └────────────┘
        │                 │
        │  GPIO8  ────────┼───────┐
        │                 │       │
        │  GPIO9 ─────────┼───┐   │
        │  (button)       │   │   │
        └─────────────────┘   │   │
                              │   │
                           [Button]│
                              │   │
                             GND  │
                                  │
                           ┌──────▼───────┐
                           │ Relay Module │
                           │              │
                           │  VCC ────────┼──── +5V
                           │  GND ────────┼──── GND
                           │  IN  (from   │
                           │      GPIO8)  │
                           │              │
                           │  NO ─────────┼──┐ (Normally Open)
                           │  COM ────────┼──┼─── to Light
                           │  NC         │  │ (Normally Closed, unused)
                           └──────────────┘  │
                                             │
                                          [Light]
                                             │
                                            AC/DC
                                           Power
```

## Detailed Wiring Instructions

### Step 1: ESP32-C3 Power
```
Power Supply +5V  →  ESP32-C3 5V/USB pin (or USB-C port)
Power Supply GND  →  ESP32-C3 GND pin
```

### Step 2: PIR Sensor Connection
```
PIR Sensor VCC  →  +5V (from power supply or ESP32-C3 5V pin*)
PIR Sensor GND  →  GND (common ground)
PIR Sensor OUT  →  ESP32-C3 GPIO10

*Note: If ESP32-C3 5V pin can't provide enough current, use external 5V
```

**PIR Sensor Settings (on sensor board):**
- Set jumper to "Repeatable Trigger" mode (H position)
- Adjust sensitivity potentiometer as needed
- Time delay potentiometer can be set to minimum (code handles timing)

### Step 3: Relay Module Connection
```
Relay Module VCC  →  +5V power supply
Relay Module GND  →  GND (common ground)
Relay Module IN   →  ESP32-C3 GPIO8
```

**Important:** Most relay modules are **active LOW**, meaning:
- GPIO8 = HIGH (3.3V) → Relay OFF
- GPIO8 = LOW (0V) → Relay ON

If you need active HIGH logic, modify `pir_light_control.cpp`:
```cpp
// Line where GPIO is set
gpio_set_level(g_config.light_gpio, on ? 1 : 0);  // Current (active HIGH)
// Change to:
gpio_set_level(g_config.light_gpio, on ? 0 : 1);  // For active LOW
```

### Step 4: Light Connection to Relay
```
AC/DC Power Source  →  Relay COM (Common)
Relay NO (Normally Open)  →  Light positive/hot wire
Light negative/neutral  →  AC/DC Power Source return
```

**For AC Lights (120V/230V):**
```
Live Wire (Hot)  →  Relay COM
Relay NO  →  Light terminal 1
Neutral Wire  →  Light terminal 2
Ground Wire  →  Light ground (do not switch ground!)
```

**For DC Lights/LEDs (12V/24V):**
```
DC+ from power supply  →  Relay COM
Relay NO  →  LED strip + terminal
LED strip - terminal  →  DC- (GND)
```

### Step 5: Optional Button for Factory Reset
```
ESP32-C3 GPIO9  →  Push Button (one side)
Push Button (other side)  →  GND

Or use internal pull-up (already configured in code):
ESP32-C3 GPIO9  →  Button  →  GND
```

## Connection Checklist

- [ ] ESP32-C3 has 5V power (via USB or external supply)
- [ ] All GND connections are common (same ground rail)
- [ ] PIR sensor VCC connected to 5V
- [ ] PIR sensor OUT connected to GPIO10
- [ ] PIR sensor set to "Repeatable Trigger" mode
- [ ] Relay module VCC connected to 5V
- [ ] Relay module IN connected to GPIO8
- [ ] Relay module COM and NO connected to light circuit
- [ ] Light power supply is separate from logic supply
- [ ] No AC voltage touching ESP32 pins
- [ ] Optional button connected between GPIO9 and GND

## Power Supply Considerations

### Option 1: USB Power (Simple)
- Use USB-C cable to power ESP32-C3
- ESP32-C3 provides 5V to PIR and Relay
- **Limitation**: Max ~500mA from USB (usually sufficient for relay module)

### Option 2: External 5V Supply (Recommended)
- 5V 2A power adapter
- Connect to breadboard power rails
- ESP32-C3, PIR, and Relay all powered from same 5V supply
- **Advantage**: More stable, higher current capacity

### Option 3: Separate Power (Advanced)
- ESP32-C3: USB power (5V)
- PIR Sensor: 5V from external supply
- Relay: 5V from external supply
- **Must connect all GNDs together!**

## PCB Pin Labels (ESP32-C3 Supermini)

```
         USB-C Port
┌─────────────────────────┐
│ [O]  3V3  G  IO0  IO1  │  IO = GPIO
│ [O]  IO2  IO3  IO4  IO5│
│ [O]  IO6  IO7  IO8  IO9│  ← GPIO8 (light)
│ [O]  IO10 IO20 IO21    │  ← GPIO10 (PIR)
│ [O]  RX  TX  5V  G     │  ← GPIO9 (button)
└─────────────────────────┘
         Bottom View
```

**Our Connections:**
- **GPIO10** (PIR) - Right side, 4th pin from bottom
- **GPIO8** (Light) - Right side, 6th pin from bottom  
- **GPIO9** (Button) - Right side, 5th pin from bottom
- **5V** - Bottom row, 2nd from right
- **GND** - Multiple (use any G pin)

## Testing Individual Components

### Test 1: PIR Sensor
```cpp
// Upload simple test code:
void loop() {
    Serial.println(digitalRead(10));  // Read GPIO10
    delay(100);
}
```
- Should print 0 when no motion
- Should print 1 when motion detected
- PIR LED should light up during detection

### Test 2: Relay Module
```cpp
// Upload simple test code:
void loop() {
    digitalWrite(8, HIGH);  // Relay on
    delay(1000);
    digitalWrite(8, LOW);   // Relay off
    delay(1000);
}
```
- Should hear relay clicking
- LED on relay module should toggle
- Check continuity between COM and NO with multimeter

### Test 3: Full System
- Upload the full MotionLux PIR firmware
- Commission via Matter app
- Test motion detection with serial monitor
- Verify light control works

## Safety Guidelines

### ⚠️ CRITICAL SAFETY WARNINGS

1. **Never Connect AC Directly to ESP32**
   - Use relay module with proper isolation
   - AC mains can be lethal - 120V/230V AC

2. **Proper Grounding**
   - Ensure AC circuits are properly grounded
   - Don't switch the ground wire through relay

3. **Voltage Ratings**
   - Verify relay module is rated for your voltage
   - Check current rating matches or exceeds load

4. **Isolation**
   - Use optical isolation between logic and AC
   - Keep AC circuit physically separated

5. **Enclosure**
   - Put AC connections in proper enclosure
   - No exposed AC terminals

6. **Professional Installation**
   - For permanent AC installations, consult electrician
   - Follow local electrical codes

7. **Testing**
   - Test with low voltage DC loads first
   - Use multimeter to verify connections
   - Never work on live AC circuits

### 🔒 Recommended Safety Measures

- **Fuse**: Add appropriate fuse to AC circuit
- **GFCI**: Use GFCI outlet for AC lights
- **Enclosure**: IP-rated box for outdoor use
- **Wire Gauge**: Use appropriate wire gauge for current
- **Strain Relief**: Secure all connections properly
- **Labeling**: Label all terminals clearly

## Troubleshooting Wiring Issues

### PIR Not Detecting
- **Check**: PIR LED - should light when detecting
- **Check**: Voltage at PIR VCC - should be ~5V
- **Check**: Output at GPIO10 - should go HIGH with motion
- **Adjust**: Sensitivity potentiometer on PIR sensor
- **Adjust**: Set jumper to repeatable trigger mode

### Relay Not Switching
- **Check**: Voltage at relay VCC - should be ~5V
- **Check**: Signal at relay IN - should change with GPIO8
- **Listen**: Should hear audible click when switching
- **Check**: Relay LED indicator
- **Test**: Manually ground IN pin - relay should activate

### Light Not Responding
- **Check**: Continuity between relay NO and COM when activated
- **Check**: Power to light circuit is present
- **Check**: Light bulb/LED is functional
- **Check**: Relay contact rating sufficient for load
- **Verify**: Correct relay terminals used (NO, not NC)

### ESP32 Not Booting
- **Check**: USB cable is data-capable (not charge-only)
- **Check**: 5V voltage at ESP32 power pin
- **Check**: No short circuits between pins
- **Check**: Button not stuck pressed during boot
- **Reset**: Press BOOT + RST buttons to enter flash mode

### All Components Get Power But Nothing Works
- **Check**: All grounds are connected together
- **Check**: GPIO pins match code configuration
- **Reflash**: Upload firmware again with correct target
- **Monitor**: Check serial output for error messages

## Example Breadboard Layout

```
Power Rails:
[+5V]  ████████████████████████  Red rail
[GND]  ████████████████████████  Blue rail

Components:
                PIR Sensor
                ┌─────────┐
                │  [O]    │ VCC → +5V rail
                │  [O]    │ GND → GND rail
                │  [O]    │ OUT → GPIO10
                └─────────┘

ESP32-C3                     Relay Module
┌──────────┐                ┌───────────┐
│          │                │  [O] VCC  │ → +5V rail
│ GPIO10 [O]───────────────→│  [O] GND  │ → GND rail
│ GPIO8  [O]───────────────→│  [O] IN   │
│ GPIO9  [O]────┐           │           │
│ 5V     [O]────┼─→ +5V rail│  COM  NO  │ → to Light
│ GND    [O]────┼─→ GND rail└───────────┘
└──────────┘    │
              [BTN]
                │
               GND
```

## Alternative: Direct LED Control (No Relay)

For testing or low-power LEDs (< 20mA):

```
ESP32-C3 GPIO8  →  [220Ω Resistor]  →  LED Anode (+)
LED Cathode (-)  →  GND

Note: Only for small indicator LEDs, not for lighting applications
```

## Advanced: MOSFET Circuit (Alternative to Relay)

For DC loads without relay clicking noise:

```
ESP32-C3 GPIO8  →  [10kΩ Resistor]  →  MOSFET Gate
MOSFET Source  →  GND
MOSFET Drain  →  LED/Light negative (-)
LED/Light positive (+)  →  Power supply +12V/24V
```

**Recommended MOSFET:** IRL540N (logic-level, N-channel)

## Questions & Answers

**Q: Can I use 3.3V for PIR sensor?**
A: Most PIR sensors work on 3.3V-5V. Check your sensor datasheet. 5V is recommended for better reliability.

**Q: Do I need pull-up resistors?**
A: No, they're configured in software. GPIO10 has internal pull-up enabled.

**Q: Can I control multiple lights?**
A: Yes, add more relay modules and modify code to control additional GPIOs.

**Q: What if my relay is active LOW?**
A: Invert the logic in code: `gpio_set_level(pin, on ? 0 : 1);`

**Q: Can I use with LED strip?**
A: Yes, ensure relay is rated for LED current, or use MOSFET circuit.

**Q: Is optical isolation necessary?**
A: Highly recommended for AC applications. Most relay modules include it.

## Additional Resources

- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [HC-SR501 PIR Sensor Datasheet](https://www.epitran.it/ebayDrive/datasheet/44.pdf)
- [Relay Module Tutorial](https://www.circuitbasics.com/setting-up-a-relay-on-the-arduino/)
- ESP32 GPIO Reference

---

**Safety First:** When in doubt, consult a qualified electrician for AC installations.
