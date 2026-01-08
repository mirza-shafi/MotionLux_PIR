# Build & Flash Guide - MotionLux PIR

## Prerequisites Check

Before building, ensure you have:
- ✅ ESP-IDF v5.1 or later installed
- ✅ ESP32-C3 board connected via USB
- ✅ Drivers installed (CH340, CP2102, or ESP32-C3 USB-Serial)
- ✅ Python 3.7 or later
- ✅ Project downloaded/cloned

## Step-by-Step Build Process

### 1. Open Terminal/Command Prompt

**Linux/macOS:**
```bash
# Open terminal in project directory
cd /Users/mirzashafi/esp/MotionLux_PIR
```

**Windows:**
```cmd
# Open ESP-IDF command prompt
cd C:\Users\YourName\esp\MotionLux_PIR
```

### 2. Set Up ESP-IDF Environment

**Linux/macOS:**
```bash
# Source ESP-IDF environment
. $HOME/esp/esp-idf/export.sh

# Or if you installed elsewhere:
. /path/to/esp-idf/export.sh
```

**Windows:**
```cmd
# Run ESP-IDF environment script
%USERPROFILE%\esp\esp-idf\export.bat

# Or use ESP-IDF PowerShell/CMD shortcut
```

**Expected Output:**
```
Setting up ESP-IDF environment...
Done! You can now compile ESP-IDF projects.
```

### 3. Set Target to ESP32-C3

```bash
idf.py set-target esp32c3
```

**Expected Output:**
```
Set target to esp32c3
Running cmake...
Build directory: /path/to/MotionLux_PIR/build
...
Project configuration is complete.
```

**⚠️ Important:** Run this only once, or when switching targets.

### 4. Configure Project (Optional)

```bash
idf.py menuconfig
```

Navigate using arrow keys:
- **Example Configuration** → Customize GPIO pins and timeout
- **Component config** → Advanced settings
- Press `Q` to quit, `Y` to save

**Default settings are already configured for your requirements!**

### 5. Build the Project

```bash
idf.py build
```

**Build Time:** 2-5 minutes (first time), 30-60 seconds (subsequent builds)

**Expected Output (Success):**
```
[100%] Built target app
Project build complete. To flash, run:
 idf.py flash
or
 python -m esptool --chip esp32c3 ...
```

**If Build Fails:**
```bash
# Clean and retry
idf.py fullclean
idf.py build
```

### 6. Identify ESP32-C3 USB Port

**Linux:**
```bash
# List USB devices
ls /dev/ttyUSB* /dev/ttyACM*

# Common: /dev/ttyUSB0 or /dev/ttyACM0
```

**macOS:**
```bash
# List USB devices
ls /dev/tty.*

# Common: /dev/tty.usbserial-* or /dev/tty.SLAB_USBtoUART
```

**Windows:**
```cmd
# Open Device Manager
# Look under "Ports (COM & LPT)"
# Common: COM3, COM4, COM5
```

### 7. Flash to ESP32-C3

**Auto-detect port:**
```bash
idf.py flash
```

**Specify port:**
```bash
# Linux
idf.py -p /dev/ttyUSB0 flash

# macOS
idf.py -p /dev/tty.usbserial-14120 flash

# Windows
idf.py -p COM3 flash
```

**Expected Output:**
```
Detecting chip type... ESP32-C3
...
Writing at 0x00010000... (100%)
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
Done
```

### 8. Open Serial Monitor

```bash
# Flash and immediately monitor
idf.py -p /dev/ttyUSB0 flash monitor

# Or just monitor
idf.py -p /dev/ttyUSB0 monitor
```

**Exit Monitor:** Press `Ctrl+]`

## Serial Monitor Output (What to Expect)

### Successful Boot:
```
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
...
I (123) main_task: Calling app_main()
I (345) pir_light: PIR Light Control initialized: PIR=GPIO10, Light=GPIO8, Timeout=120000ms
I (346) app_main: PIR Light Control initialized successfully
I (500) chip[DL]: BLE Host Task Started
I (600) chip[DL]: CHIPoBLE advertising started
```

### Look for QR Code:
```
I (1234) chip[SVR]: Server Listening...
I (1250) chip[SVR]: SetupQRCode: [MT:Y.K9042C00KA0648G00]
I (1260) chip[SVR]: Copy/paste the below URL in a browser to see the QR Code:
I (1270) chip[SVR]: https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT:Y.K9042C00KA0648G00
```

**⭐ COPY THIS QR CODE! ⭐**

### PIR Activity:
```
I (15000) app_driver: Changing light power to 1
I (15001) pir_light: Light turned ON
I (15002) pir_light: PIR sensor control ENABLED
I (18523) pir_light: Motion detected, timer reset (light stays ON)
I (22156) pir_light: Motion detected, timer reset (light stays ON)
I (138002) pir_light: Inactivity timeout: turning light OFF (PIR controlled)
I (138003) pir_light: PIR sensor control DISABLED
```

## Troubleshooting Build/Flash Issues

### Issue: Command not found: idf.py

**Solution:**
```bash
# ESP-IDF environment not set up
. $HOME/esp/esp-idf/export.sh

# Or on Windows:
%USERPROFILE%\esp\esp-idf\export.bat
```

### Issue: Serial port not found

**Solution:**
```bash
# Check USB cable (must be data cable, not charge-only)
# Install drivers (CH340/CP2102)
# Try different USB port
# Check Device Manager (Windows) or dmesg (Linux)

# Linux: Add user to dialout group
sudo usermod -a -G dialout $USER
# Then log out and back in
```

### Issue: Permission denied on Linux/macOS

**Solution:**
```bash
# Give permission to USB port
sudo chmod 666 /dev/ttyUSB0

# Or add user to dialout group (permanent)
sudo usermod -a -G dialout $USER
```

### Issue: Failed to connect to ESP32-C3

**Solution:**
1. Press and hold BOOT button
2. Press and release RESET button
3. Release BOOT button
4. Try flashing again:
```bash
idf.py -p /dev/ttyUSB0 flash
```

### Issue: Build fails with CMake error

**Solution:**
```bash
# Clean everything
rm -rf build
idf.py fullclean

# Set target again
idf.py set-target esp32c3

# Build
idf.py build
```

### Issue: No QR code in serial monitor

**Solution:**
1. Wait 30-60 seconds after boot
2. Press RESET button on ESP32-C3
3. Check baud rate (115200 default)
4. Try:
```bash
idf.py monitor -b 115200
```

### Issue: Brownout detector error

**Solution:**
```
# Power supply insufficient
# Use USB 2.0 port (more power than USB 3.0 sometimes)
# Use external 5V power supply
# Try different USB cable
```

## Quick Commands Reference

```bash
# Initial setup (run once)
idf.py set-target esp32c3

# Build only
idf.py build

# Flash only
idf.py flash

# Flash and monitor
idf.py flash monitor

# Monitor only
idf.py monitor

# Clean build
idf.py fullclean

# Configure
idf.py menuconfig

# Erase flash (factory reset)
idf.py erase-flash

# Get flash size info
idf.py flash-info

# Specify port
idf.py -p /dev/ttyUSB0 [command]

# Change baud rate
idf.py -b 921600 flash
```

## VS Code Integration (Optional)

If using VS Code with ESP-IDF extension:

1. Open project folder in VS Code
2. Press `Ctrl+Shift+P` (Cmd+Shift+P on Mac)
3. Type: "ESP-IDF: Set Espressif device target"
4. Select: "esp32c3"
5. Use buttons at bottom:
   - 🔨 Build
   - ⚡ Flash
   - 📊 Monitor
   - 🔧 Build, Flash & Monitor

## USB-JTAG Debugging (Advanced)

ESP32-C3 has built-in USB-JTAG:

```bash
# OpenOCD with ESP32-C3
openocd -f board/esp32c3-builtin.cfg

# In another terminal, run GDB
xtensa-esp32-elf-gdb build/MotionLux_PIR.elf
```

## Binary File Location

After successful build:

```
build/
├── MotionLux_PIR.bin        ← Main firmware
├── bootloader/
│   └── bootloader.bin       ← Bootloader
├── partition_table/
│   └── partition-table.bin  ← Partition table
└── MotionLux_PIR.elf        ← Debug symbols
```

## Flash Manually (Advanced)

```bash
# Using esptool.py
python -m esptool --chip esp32c3 \
  --port /dev/ttyUSB0 \
  --baud 921600 \
  --before default_reset \
  --after hard_reset \
  write_flash -z \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size detect \
  0x0 build/bootloader/bootloader.bin \
  0x8000 build/partition_table/partition-table.bin \
  0x10000 build/MotionLux_PIR.bin
```

## Verify Flash Success

After flashing, check serial output for:

```
✅ "PIR Light Control initialized successfully"
✅ "Server Listening..."
✅ "SetupQRCode: [MT:...]"
✅ No error messages
✅ No crash loops (continuous rebooting)
```

## Build Statistics

Typical build output:

- **Binary Size**: ~1.2 MB
- **Flash Usage**: ~1.5 MB / 4 MB (37%)
- **RAM Usage**: ~120 KB / 400 KB (30%)
- **Build Time**: 2-5 minutes (first), 30s (incremental)

## Post-Flash Steps

1. ✅ Note the QR code from serial monitor
2. ✅ Save QR code for commissioning
3. ✅ Test serial output shows PIR initialization
4. ✅ Proceed to Matter commissioning
5. ✅ Test motion detection functionality

## Common Serial Messages

| Message | Meaning |
|---------|---------|
| `PIR Light Control initialized` | ✅ System ready |
| `Motion detected, timer reset` | ✅ PIR working |
| `PIR sensor control ENABLED` | ✅ PIR active |
| `Light turned ON` | ✅ Light control working |
| `Commissioning complete` | ✅ Matter paired |
| `Guru Meditation Error` | ❌ Crash (check wiring) |
| `Brownout detector` | ❌ Power issue |

## Need Help?

1. **Build errors**: Check ESP-IDF version (`idf.py --version`)
2. **Flash errors**: Check USB cable and drivers
3. **Runtime errors**: Check serial monitor for details
4. **Wiring issues**: See WIRING_GUIDE.md
5. **Setup questions**: See PIR_SETUP_GUIDE.md

## Ready to Commission?

After successful flash and QR code noted:

➡️ **Next**: Open Matter app and scan QR code  
📖 **Guide**: See PIR_SETUP_GUIDE.md → Matter Commissioning section

---

**Build Environment**: ESP-IDF 5.1+  
**Target**: ESP32-C3  
**Project**: MotionLux PIR  
**Status**: Ready to flash
