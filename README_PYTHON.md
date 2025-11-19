# MakcuFlasher - Python Version (ESP32)

Simple Python-based firmware uploader for ESP32-based Makcu devices.

## Quick Start

### 1. Install Dependencies

```bash
pip3 install esptool
```

### 2. Run Interactive Mode (Recommended)

**Linux:**
```bash
python3 makcu_flash.py
```

**Windows:**
```powershell
python makcu_flash.py
```

The program will:
1. Auto-detect all connected ESP32 devices
2. Show available firmware files
3. Let you select with numbers

### 3. Manual Mode

```bash
# Linux
python3 makcu_flash.py /dev/ttyACM0 firmware/V3.8.bin

# Windows
python makcu_flash.py COM3 firmware\V3.8.bin
```

## Firmware Files

All firmware files (V2.0 - V3.8) are included in the `firmware/` directory.

## Linux Permissions

If you get permission errors:

```bash
sudo usermod -a -G dialout $USER
# Log out and log back in
```

Or run with sudo (not recommended):
```bash
sudo python3 makcu_flash.py
```

## Troubleshooting

### "esptool.py not found"
```bash
pip3 install esptool
```

### "No serial ports detected"
- Check device connection
- On Linux: `ls /dev/ttyUSB* /dev/ttyACM*`
- Check permissions (dialout group)

### ModemManager interference (Linux)
```bash
sudo systemctl stop ModemManager
sudo systemctl disable ModemManager
```

## C++ Version

For a standalone C++ version, see the main README and use the compiled `MakcuFlasher` binary (note: currently uses generic bootloader protocol, may not work with ESP32).
