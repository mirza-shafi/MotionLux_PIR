## MotionLux PIR - Smart Motion-Controlled Matter Light

## Overview
MotionLux PIR is a Matter-enabled smart light system for ESP32-C3 that uses a PIR (Passive Infrared) motion sensor for intelligent automatic lighting control. The light is controlled via Matter protocol (compatible with Google Home, Apple Home, Samsung SmartThings, etc.) and features automatic turn-off after 2 minutes of inactivity when no motion is detected.

## Key Features
- **Matter Protocol Support** - Works with all major smart home platforms
- **PIR Motion Detection** - Keeps light ON as long as motion is detected
- **Smart Timer** - Automatically turns OFF after 2 minutes of no movement
- **App-Controlled** - Manual ON/OFF control via Matter app
- **Multi-User Support** - QR code commissioning for multiple users
- **PIR Enable/Disable** - PIR sensor only active when light is manually turned ON
- **ESP32-C3 Optimized** - Low power consumption, compact design

## Hardware Requirements
- **ESP32-C3 Supermini** microcontroller
- **PIR Motion Sensor** (HC-SR501 or similar)
- **Relay Module** or transistor for light control
- **Light/LED** (compatible with your relay module)
- Connecting wires

### Pin Configuration
| Component | GPIO Pin |
|-----------|----------|
| PIR Sensor Output | GPIO 10 |
| Light Control | GPIO 8 |
| Button (Optional) | GPIO 9 |

## How It Works

1. **Turn ON via App** - Use your Matter app to turn the light ON
2. **PIR Activates** - Motion sensor becomes active and starts monitoring
3. **Motion Detected** - Any movement resets the 2-minute timer
4. **No Motion** - If no movement for 2 minutes, light turns OFF automatically
5. **Turn OFF via App** - Manual OFF disables PIR sensor

### Motion Detection Logic
- Every detected motion resets the inactivity timer to zero
- Even small movements (hand gesture, person shifting) keep the light ON
- Light only turns off after 2 full minutes of complete stillness
- PIR control only works when light is manually turned ON via app

## Features
- Matter protocol integration for universal smart home compatibility
- Automatic light control based on room occupancy
- Configurable timeout period (default: 2 minutes)
- Multi-user access through QR code commissioning
- Factory reset support
- OTA (Over-the-Air) firmware updates
- Secure certificate management
- Thread and Wi-Fi support

## Directory Structure
- `main/` - Main application source code
  - `app_main.cpp` - Main entry point and PIR initialization
  - `app_driver.cpp` - Light driver and Matter integration
  - `pir_light_control.cpp/h` - PIR sensor and light control logic
  - `app_priv.h` - Private header definitions
  - `CMakeLists.txt` - Build configuration for main app
  - `idf_component.yml` - Component manifest
  - `Kconfig.projbuild` - Project configuration options (GPIO pins, timeout)
  - `certification_declaration/` - Matter certification files
- `managed_components/` - ESP-IDF managed components
- `PIR_SETUP_GUIDE.md` - **Detailed setup and usage instructions**
- `CMakeLists.txt` - Top-level build configuration
- `partitions.csv` - Partition table for flash layout
- `sdkconfig*` - SDK configuration files for various chipsets

## Getting Started

> **📖 For detailed setup instructions, wiring diagrams, and troubleshooting, see [PIR_SETUP_GUIDE.md](PIR_SETUP_GUIDE.md)**

### Prerequisites
- ESP-IDF v5.1 or later ([Setup Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/))
- ESP32-C3 Supermini board
- PIR motion sensor module
- Relay module or LED for testing
- Python 3.x
- Matter-compatible smart home app (Google Home, Apple Home, etc.)

### Quick Start
1. **Set up ESP-IDF environment:**
    ```sh
    . $HOME/esp/esp-idf/export.sh
    ```

2. **Set target to ESP32-C3:**
    ```sh
    idf.py set-target esp32c3
    ```

3. **Configure the project (optional):**
    ```sh
    idf.py menuconfig
    # Navigate to "Example Configuration" to customize GPIO pins and timeout
    ```

4. **Build the project:**
    ```sh
    idf.py build
    ```

5. **Flash to ESP32-C3:**
    ```sh
    idf.py -p /dev/ttyUSB0 flash monitor
    # Replace /dev/ttyUSB0 with your actual port
    # On macOS: /dev/tty.usbserial-*
    # On Windows: COM3, COM4, etc.
    ```

6. **Commission with Matter app:**
    - Look for QR code in serial monitor output
    - Open your Matter app (Google Home, Apple Home, etc.)
    - Scan the QR code to add the device
    - Complete the commissioning process

7. **Test the system:**
    - Turn light ON via app → PIR sensor activates
    - Wave hand in front of sensor → Timer resets
    - Wait 2 minutes without motion → Light turns OFF
    - Turn light OFF via app → PIR sensor deactivates

## Configuration Options

Available in `idf.py menuconfig` under **Example Configuration**:

| Option | Default | Description |
|--------|---------|-------------|
| PIR Sensor GPIO | 10 | GPIO pin for PIR sensor input |
| Light Control GPIO | 8 | GPIO pin for light/relay output |
| PIR Timeout | 120 seconds | Inactivity timeout before auto-off |

## Matter Commissioning

### QR Code Location
After flashing, find the commissioning QR code in the serial monitor:
```
[Matter] Setup QR code: MT:Y.K9042C00KA0648G00
```

### Multi-User Access
The same QR code can be used by multiple users:
1. First user commissions the device
2. Additional users can:
   - Be invited through the app's sharing feature
   - Scan the same QR code during commissioning window
   - Access the device on the same network

Matter supports up to 16 simultaneous controllers (users/apps).

## Troubleshooting

### Common Issues

**PIR sensor not working:**
- Verify wiring connections (VCC, GND, OUT to GPIO10)
- Check PIR sensor power LED
- Adjust PIR sensitivity potentiometer
- Enable light via app first

**Light not responding:**
- Check GPIO8 connection to relay/LED
- Verify relay module power supply
- Check serial monitor for errors
- Test relay with direct wire connection

**Matter commissioning fails:**
- Ensure Bluetooth is enabled on phone
- Check Wi-Fi credentials
- Verify Matter app is updated
- Try factory reset (hold button 5 seconds)

**Timer not working:**
- Ensure light is ON via app
- Check serial monitor for "Motion detected" messages
- Verify 2-minute timeout setting
- Confirm PIR sensor is detecting (test LED on sensor)

For detailed troubleshooting, see [PIR_SETUP_GUIDE.md](PIR_SETUP_GUIDE.md).

## Project Structure

```
MotionLux_PIR/
├── main/
│   ├── app_main.cpp              # Matter initialization & PIR setup
│   ├── app_driver.cpp            # Light control & Matter callbacks  
│   ├── pir_light_control.cpp     # PIR sensor logic & timer
│   ├── pir_light_control.h       # PIR API definitions
│   └── Kconfig.projbuild         # Configuration options
├── managed_components/            # ESP-IDF managed dependencies
├── PIR_SETUP_GUIDE.md            # Comprehensive setup guide
├── README.md                     # This file
└── sdkconfig.defaults.esp32c3    # ESP32-C3 default config
```

## License

This project is provided as-is under public domain or CC0 license. Feel free to use, modify, and distribute.

## Support

For issues or questions:
- Check [PIR_SETUP_GUIDE.md](PIR_SETUP_GUIDE.md) for detailed documentation
- Review serial monitor output for error messages
- Verify hardware connections
- Consult ESP-IDF documentation

## Safety Warning

⚠️ When controlling AC-powered lights:
- Use appropriate relay module rated for your voltage
- Never connect AC mains directly to ESP32
- Use optical isolation
- Follow local electrical codes
- Consider professional installation for AC applications
4. **Flash to device:**
    ```sh
    idf.py -p <PORT> flash
    ```
    Replace `<PORT>` with your device's serial port (e.g., `/dev/ttyUSB0`).

### Partition Table
The `partitions.csv` file defines the flash memory layout. You can customize it as needed for your application.

### Configuration
- Use the appropriate `sdkconfig.defaults.*` file for your target chipset and features.
- Additional configuration can be done via `menuconfig`.

## Components
This project uses several ESP-IDF managed components, including:
- `button` - Button handling
- `cbor` - CBOR encoding/decoding
- `esp_diag_data_store` - Diagnostics data storage
- `esp_diagnostics` - Diagnostics framework
- `esp_encrypted_img` - Encrypted image support
- `esp_insights` - Insights and telemetry
- `esp_rcp_update` - RCP update support
- `esp_secure_cert_mgr` - Secure certificate management
- `esp-serial-flasher` - Serial flashing utility
- `jsmn`, `json_generator`, `json_parser` - JSON support
- `led_strip` - LED strip control
- `mdns` - mDNS support
- `rmaker_common` - RainMaker common utilities

## Certification
Certification-related files are located in `main/certification_declaration/`.

## Contributing
1. Fork the repository and create your branch.
2. Commit your changes with clear messages.
3. Push to your fork and submit a pull request.

## License
See the `LICENSE` file for license information.

## References
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [ESP-IDF Component Registry](https://components.espressif.com/)

---
For more details, refer to the source code and comments within each file.
