# MakcuFlasher

Cross-platform firmware uploader for ESP32-based Makcu devices.

Simple Python wrapper around esptool for easy firmware flashing with interactive mode.

---

## Features

- **Auto-detect ESP32 devices**: Automatically finds connected Makcu boards
- **Interactive mode**: Just run without arguments - select device and firmware with numbers
- **Bundled firmware**: All firmware versions (V2.0 - V3.8) included
- **Cross-platform**: Works on Windows, Linux, and macOS
- **Easy to use**: No complex setup or compilation needed

---

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

### Interactive Session Example

```
==================================================
  MakcuFlasher - Interactive Mode
==================================================

Available serial ports:
  1. /dev/ttyACM0

Select port (1-1) or enter manually: 1

Available firmware files:
  1. firmware/V2.0.bin
  2. firmware/V3.0.bin
  3. firmware/V3.2.bin
  4. firmware/V3.4.bin
  5. firmware/V3.7.bin
  6. firmware/V3.8.bin

Select firmware (1-6) or enter path: 6
```

---

## Usage

### Interactive Mode (Recommended)

Just run without arguments:

```bash
python3 makcu_flash.py
```

### Manual Mode

Specify port and firmware file:

**Linux:**
```bash
python3 makcu_flash.py /dev/ttyACM0 firmware/V3.8.bin
```

**Windows:**
```powershell
python makcu_flash.py COM3 firmware\V3.8.bin
```

---

## Firmware Files

All firmware versions are included in the `firmware/` directory:
- V2.0.bin
- V3.0.bin
- V3.2.bin
- V3.4.bin
- V3.7.bin
- V3.8.bin

No need to download separately!

---

## Installation

### Requirements

- Python 3.6 or higher
- esptool

### Install esptool

```bash
pip3 install esptool
```

### Clone Repository

```bash
git clone https://github.com/needitem/MakcuFlasher.git
cd MakcuFlasher
```

### Install Dependencies

You can install dependencies using `requirements.txt`:

```bash
pip3 install -r requirements.txt
```

Or install the package in editable mode:

```bash
pip3 install -e .
```

---

## Linux Permissions

If you get permission errors:

### Option 1: Add user to dialout group (Recommended)

```bash
sudo usermod -a -G dialout $USER
# Log out and log back in for the change to take effect
```

### Option 2: Run with sudo (Not recommended)

```bash
sudo python3 makcu_flash.py
```

### Option 3: Temporary permission

```bash
sudo chmod 666 /dev/ttyACM0
python3 makcu_flash.py
```

---

## Troubleshooting

### "esptool.py not found"

Install esptool:
```bash
pip3 install esptool
```

### "No serial ports detected"

- Check that your Makcu device is connected via USB
- On Linux: Check available ports with `ls /dev/ttyUSB* /dev/ttyACM*`
- On Windows: Check Device Manager for COM ports
- Verify USB cable supports data transfer (not just charging)

### Permission denied (Linux)

Add your user to the dialout group:
```bash
sudo usermod -a -G dialout $USER
```
Then log out and log back in.

### ModemManager interference (Linux)

ModemManager may interfere with serial ports:
```bash
sudo systemctl stop ModemManager
sudo systemctl disable ModemManager
```

### Flashing fails or disconnects

- Ensure USB cable is good quality and properly connected
- Try a different USB port
- Hold the BOOT button on the device while connecting (if available)
- Lower the baud rate by editing `makcu_flash.py` and changing `460800` to `115200`

---

## Technical Details

### ESP32 Chip

Makcu devices use ESP32 microcontrollers. The firmware flashing process:

1. **Connect**: Device enters ROM bootloader mode
2. **Erase**: Flash memory is erased
3. **Write**: Firmware is written to flash at address 0x0
4. **Verify**: Data integrity is checked
5. **Reset**: Device resets and runs new firmware

### Flash Parameters

- **Chip**: ESP32
- **Baud Rate**: 460800 (can be lowered to 115200 if needed)
- **Flash Mode**: DIO (Dual I/O)
- **Flash Frequency**: 40MHz
- **Flash Address**: 0x0

---

## Related Projects

- [MakcuRelay](https://github.com/needitem/MakcuRelay) - UDP relay for Makcu-based 2PC aimbot setups

---

## License

This project is released under the MIT License.

---

## Credits

Built with:
- [esptool](https://github.com/espressif/esptool) - Espressif's official ESP32 flash tool
- Python 3

🤖 Generated with [Claude Code](https://claude.com/claude-code)
