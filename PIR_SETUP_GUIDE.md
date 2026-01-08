# PIR Motion Sensor Matter Light - Setup Guide

## Hardware Configuration

### ESP32-C3 Supermini Connections

| Component | GPIO Pin | Description |
|-----------|----------|-------------|
| PIR Sensor Output | GPIO 10 | Motion detection signal input |
| Light Control | GPIO 8 | Light relay/LED control output |
| Button (Optional) | GPIO 9 | Factory reset and toggle button |

### PIR Sensor Wiring
- **VCC** → 3.3V or 5V (check your PIR sensor specs)
- **GND** → Ground
- **OUT** → GPIO 10 (ESP32-C3)

### Light Control Wiring
- **GPIO 8** → Relay module IN or LED+ (through appropriate driver circuit)
- **GND** → Common ground

## Functional Behavior

### Motion Detection Logic

1. **Light must be turned ON via Matter app first**
   - PIR sensor only becomes active when you manually turn the light ON through the app
   - When light is OFF, PIR sensor is disabled

2. **Motion Detection**
   - When PIR detects any motion (even small movements like blinking, mouse movements)
   - The 2-minute timer resets to zero
   - Light stays ON

3. **Auto Turn-Off**
   - If NO motion is detected for 2 minutes continuously
   - Light automatically turns OFF
   - PIR sensor control is disabled

4. **Manual Control**
   - You can always turn light ON/OFF via Matter app
   - Turning ON enables PIR control
   - Turning OFF disables PIR control

## Building and Flashing

### Prerequisites
1. ESP-IDF installed (v5.1 or later recommended)
2. VS Code with ESP-IDF extension
3. ESP32-C3 USB drivers installed

### Build Commands

```bash
# Set target to ESP32-C3
idf.py set-target esp32c3

# Configure project (optional - to customize GPIO pins)
idf.py menuconfig

# Build the project
idf.py build

# Flash to device
idf.py -p /dev/ttyUSB0 flash monitor
```

### Configuration Options

You can customize settings via `idf.py menuconfig`:

Navigate to: **Example Configuration**

- **PIR Sensor GPIO**: Default 10
- **Light Control GPIO**: Default 8  
- **PIR Timeout**: Default 120 seconds (2 minutes)

## Matter Commissioning

### First-Time Setup

1. **Flash the firmware** to your ESP32-C3
2. **View QR code**: Check the serial monitor output for commissioning QR code
3. **Open Matter app** (Google Home, Apple Home, Samsung SmartThings, etc.)
4. **Scan the QR code** shown in serial monitor
5. **Complete setup** following app instructions
6. **Control your light** - Turn ON to enable PIR sensor

### QR Code Location

After flashing, the QR code appears in the serial monitor output:
```
[Matter] Setup QR code: MT:Y.K9042C00KA0648G00
```

You can also find it printed at startup.

### Adding Additional Users

The Matter protocol supports multiple users accessing the same device:

1. **First user** commissions the device using the QR code
2. **Additional users** can be added through:
   - **Shared fabric**: First user shares access through their app's sharing feature
   - **Same network**: Users on same network can discover and control the device
   - **QR code reuse**: The same QR code can be used multiple times during commissioning window

### Multi-User Access Methods

**Method 1: App Sharing (Recommended)**
- First user completes commissioning
- Uses app's "Share" or "Invite" feature to add family members
- No QR code needed for additional users

**Method 2: Commissioning Window**
- Open commissioning window from app or button press
- Second user scans the SAME QR code
- Device accepts multiple fabrics (up to 16 users/controllers)

**Method 3: Multi-Admin Feature**
- Matter supports multi-admin by default
- Each controller gets its own fabric on the device
- All users have equal control

## Testing the System

### Step-by-Step Test

1. **Power on ESP32-C3** and wait for Matter connection
2. **Open Matter app** and ensure device shows as "OFF"
3. **Turn light ON** via app
   - Serial monitor should show: "PIR sensor control ENABLED"
4. **Wave hand in front of PIR sensor**
   - Serial monitor should show: "Motion detected, timer reset"
5. **Stay still for 2 minutes**
   - After 2 minutes: Light turns OFF automatically
6. **Turn light OFF via app**
   - Serial monitor should show: "PIR sensor control DISABLED"

### Expected Serial Output

```
I (12345) pir_light: PIR Light Control initialized: PIR=GPIO10, Light=GPIO8, Timeout=120000ms
I (12350) app_main: PIR Light Control initialized successfully
I (15000) app_driver: Changing light power to 1
I (15001) pir_light: Light turned ON
I (15002) pir_light: PIR sensor control ENABLED
I (18000) pir_light: Motion detected, timer reset (light stays ON)
I (25000) pir_light: Motion detected, timer reset (light stays ON)
I (145000) pir_light: Inactivity timeout: turning light OFF (PIR controlled)
```

## Troubleshooting

### PIR Sensor Not Detecting Motion
- Check wiring connections
- Verify PIR sensor has power LED on
- Check PIR sensor sensitivity adjustment (potentiometer)
- Verify GPIO 10 connection
- Check serial monitor for "Motion detected" messages

### Light Not Turning ON/OFF
- Verify GPIO 8 connection to relay/LED
- Check relay module power supply
- Test relay manually with jumper wire to GPIO 8
- Check app connection to device

### Timer Not Working
- Ensure light is turned ON via app first
- Verify "PIR sensor control ENABLED" in serial monitor
- Check if motion is being detected (monitor should show resets)
- Confirm 2-minute timeout is configured correctly

### Matter Commissioning Fails
- Ensure Wi-Fi credentials are correct
- Check Bluetooth is enabled on phone
- Verify Matter app is up to date
- Try factory reset (hold button for 5 seconds)
- Check commissioning window is open

### Multiple Users Can't Connect
- Verify Matter app supports multi-admin
- Try opening commissioning window manually
- Check if device has reached fabric limit (16 max)
- Ensure all users are on same Matter fabric/home

## Advanced Configuration

### Changing Timeout Duration

Edit [app_main.cpp](main/app_main.cpp#L182):

```cpp
.inactivity_timeout_ms = 120000,   // Change this value (in milliseconds)
```

Examples:
- 1 minute: `60000`
- 5 minutes: `300000`
- 10 minutes: `600000`

### Changing GPIO Pins

Edit [app_main.cpp](main/app_main.cpp#L180-L181):

```cpp
.pir_gpio = GPIO_NUM_10,    // Change PIR pin
.light_gpio = GPIO_NUM_8,   // Change light pin
```

### Adjusting Debounce

Edit [app_main.cpp](main/app_main.cpp#L183):

```cpp
.debounce_ms = 500    // Increase if sensor is too sensitive
```

## Factory Reset

Press and hold the button (GPIO 9) for 5 seconds to perform a factory reset. This will:
- Clear all Matter commissioning data
- Remove all fabrics
- Reset to factory defaults
- Restart device and open commissioning window

## Safety Notes

⚠️ **Important Safety Information**

- Use appropriate relay module rated for your light's voltage/current
- Never connect AC mains directly to ESP32 GPIO pins
- Use optical isolation for AC-powered lights
- Ensure proper grounding
- Follow local electrical codes
- Consider using a qualified electrician for AC installations

## Support and Updates

For issues or questions:
1. Check serial monitor output for error messages
2. Verify hardware connections
3. Ensure ESP-IDF version compatibility
4. Review Matter app documentation

## Version Information

- **ESP-IDF**: v5.1+ recommended
- **Matter SDK**: Included in ESP-IDF
- **Target**: ESP32-C3
- **PIR Timeout**: 2 minutes (configurable)
- **GPIO PIR**: 10
- **GPIO Light**: 8
