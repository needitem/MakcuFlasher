# MakcuFlasher

Cross-platform firmware uploader for Makcu devices.

MakcuFlasher is a Linux-compatible replacement for the Windows-only MAKCU.exe. It supports all Makcu firmware versions and works on both Windows and Linux systems.

---

## Features

- **Cross-platform**: Works on both Windows and Linux
- **Bootloader protocol**: Full firmware upload support
  - Enter bootloader mode
  - Flash erase
  - Page-by-page writing (128-byte pages)
  - Checksum verification
  - Device reset
- **Progress tracking**: Real-time progress display during upload
- **Error handling**: Clear error messages and warnings

---

## Building

MakcuFlasher supports both **Windows** and **Linux** platforms with cross-platform code using conditional compilation.

### Windows Build

Requirements:
- Windows 10/11 x64
- Visual Studio 2022 with C++ toolchain (or MinGW)
- CMake 3.10+ in `PATH`

Build steps:

```powershell
git clone https://github.com/needitem/MakcuFlasher.git
cd MakcuFlasher

# Option 1: Using the build script
build_windows.bat

# Option 2: Manual CMake
cmake -S . -B build
cmake --build build --config Release
```

The resulting executable will be at:
```text
build\Release\MakcuFlasher.exe
```

### Linux Build

Requirements:
- Linux (Ubuntu 18.04+ or similar)
- GCC/Clang with C++17 support
- CMake 3.10+
- Build essentials (`sudo apt install build-essential cmake`)

Build steps:

```bash
git clone https://github.com/needitem/MakcuFlasher.git
cd MakcuFlasher

# Make the build script executable
chmod +x build_linux.sh

# Build
./build_linux.sh

# Or manually:
mkdir -p build_linux && cd build_linux
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

The resulting executable will be at:
```text
build_linux/MakcuFlasher
```

---

## Usage

### Firmware Files

You can download the latest firmware files from the [MAKCM_v2_files repository](https://github.com/terrafirma2021/MAKCM_v2_files).

Available firmware versions:
- V2.0.bin
- V3.0.bin
- V3.2.bin
- V3.4.bin
- V3.7.bin
- V3.8.bin

### Windows

```powershell
# Upload firmware to COM3
MakcuFlasher.exe COM3 firmware_v3.8.bin

# Upload firmware to COM4
MakcuFlasher.exe COM4 V3.8.bin
```

### Linux

First, ensure you have serial port permissions:
```bash
sudo usermod -a -G dialout $USER
# Log out and log back in for the change to take effect
```

Then upload the firmware:
```bash
# Upload firmware to /dev/ttyUSB0
./MakcuFlasher /dev/ttyUSB0 firmware_v3.8.bin

# Upload firmware to /dev/ttyACM0
./MakcuFlasher /dev/ttyACM0 V3.8.bin
```

### Firmware Upload Process

The firmware upload process includes several steps:

1. **Enter Bootloader Mode** - Device is put into firmware update mode
2. **Erase Flash** - Existing firmware is erased
3. **Write Pages** - New firmware is written in 128-byte pages
4. **Verify** - Firmware checksum is verified
5. **Exit Bootloader** - Device is reset and starts running new firmware

Example output:
```text
==================================================
          MakcuFlasher - Firmware Uploader
==================================================
Serial Port:    /dev/ttyUSB0
Firmware File:  V3.8.bin
==================================================

[MakcuFlasher] Loaded 8192 bytes from V3.8.bin

[MakcuFlasher] Opened /dev/ttyUSB0 at 115200 baud (Linux termios)

**************************************************
  WARNING: Do not disconnect the device during
  the firmware upload process!
**************************************************

[MakcuFlasher] Starting firmware upload (8192 bytes)...
[MakcuFlasher] Entering bootloader mode...
[MakcuFlasher] Successfully entered bootloader mode
[MakcuFlasher] Erasing flash memory...
[MakcuFlasher] Flash erased successfully
[MakcuFlasher] Writing 64 pages...
[MakcuFlasher] Progress: 64/64 pages
[MakcuFlasher] Verifying firmware...
[MakcuFlasher] Firmware verified successfully
[MakcuFlasher] Exiting bootloader mode...
[MakcuFlasher] Device reset, bootloader exited
[MakcuFlasher] Firmware upload complete!

==================================================
  Firmware upload successful!
==================================================
```

---

## Notes

- MakcuFlasher is intentionally minimal: no GUI, no logging framework, no configuration files.
- **Cross-platform support**: The code uses conditional compilation (`#ifdef _WIN32`) to support both Windows and Linux with the same codebase.
  - **Windows**: Uses Windows API for serial communication
  - **Linux**: Uses termios for serial communication
- Error messages (e.g. COM/serial port open failures) are printed to the console.
- If the port cannot be opened, verify:
  - **Windows**: The correct COM port name (`COM3`, `COM4`, `COM5`, …).
  - **Linux**: The correct serial device (`/dev/ttyUSB0`, `/dev/ttyACM0`, …) and proper permissions (`dialout` group).
  - No other program is using that port.
  - Makcu drivers/firmware are correctly installed.
- MakcuFlasher uses 115200 baud rate for bootloader mode communication.
- During firmware upload, the device must not be disconnected or powered off.

---

## Related Projects

- [MakcuRelay](https://github.com/needitem/MakcuRelay) - UDP relay for Makcu-based 2PC aimbot setups

---

## License

This project is released under the MIT License. See `LICENSE` if present in this repository.
