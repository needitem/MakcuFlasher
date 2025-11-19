#!/usr/bin/env python3
"""
MakcuFlasher - ESP32-based Makcu firmware uploader
Simple wrapper around esptool for easy firmware flashing
"""

import sys
import os
import glob
import subprocess
from pathlib import Path

def find_serial_ports():
    """Find available serial ports"""
    ports = []

    if sys.platform == 'win32':
        # Windows COM ports
        for i in range(1, 257):
            port = f'COM{i}'
            try:
                import serial
                s = serial.Serial(port)
                s.close()
                ports.append(port)
            except:
                pass
    else:
        # Linux/Unix
        ports = glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')
        ports.sort()

    return ports

def find_firmware_files():
    """Find available firmware files"""
    search_paths = ['firmware', '../firmware', '.']
    files = []

    for path in search_paths:
        pattern = os.path.join(path, '*.bin')
        found = glob.glob(pattern)
        if found:
            files.extend(found)
            break

    files.sort()
    return files

def flash_firmware(port, firmware_file, chip='esp32'):
    """Flash firmware using esptool"""
    print(f"\n{'='*60}")
    print(f"  MakcuFlasher - ESP32 Firmware Uploader")
    print(f"{'='*60}")
    print(f"Serial Port:    {port}")
    print(f"Firmware File:  {firmware_file}")
    print(f"Chip Type:      {chip}")
    print(f"{'='*60}\n")

    if not os.path.exists(firmware_file):
        print(f"[ERROR] Firmware file not found: {firmware_file}")
        return False

    file_size = os.path.getsize(firmware_file)
    print(f"[INFO] Firmware size: {file_size:,} bytes\n")

    print("*" * 60)
    print("  WARNING: Do not disconnect the device during")
    print("  the firmware upload process!")
    print("*" * 60)
    print()

    # Check for esptool dependency
    try:
        import esptool
    except ImportError:
        print(f"\n{'='*60}")
        print("  [ERROR] esptool not found!")
        print(f"{'='*60}")
        print("  Please install the required dependencies:")
        print("    pip3 install -r requirements.txt")
        print("    # or")
        print("    pip3 install esptool")
        print(f"{'='*60}\n")
        return False

    # esptool command - try both esptool.py and python -m esptool
    cmd_variants = [
        # Try esptool.py first (if installed as script)
        [
            'esptool.py',
            '--chip', chip,
            '--port', port,
            '--baud', '460800',
            '--before', 'default_reset',
            '--after', 'hard_reset',
            'write_flash',
            '-z',
            '--flash_mode', 'dio',
            '--flash_freq', '40m',
            '--flash_size', 'detect',
            '0x0', firmware_file
        ],
        # Fallback to python -m esptool (if installed as module)
        [
            sys.executable, '-m', 'esptool',
            '--chip', chip,
            '--port', port,
            '--baud', '460800',
            '--before', 'default_reset',
            '--after', 'hard_reset',
            'write_flash',
            '-z',
            '--flash_mode', 'dio',
            '--flash_freq', '40m',
            '--flash_size', 'detect',
            '0x0', firmware_file
        ]
    ]

    for cmd in cmd_variants:
        try:
            result = subprocess.run(cmd, check=True)
            print(f"\n{'='*60}")
            print("  Firmware upload successful!")
            print(f"{'='*60}")
            return True
        except FileNotFoundError:
            continue
        except subprocess.CalledProcessError as e:
            print(f"\n{'='*60}")
            print("  Firmware upload failed!")
            print(f"{'='*60}")
            return False

    print("[ERROR] esptool could not be executed.")
    print("  Please ensure it is installed correctly.")
    return False

def interactive_mode():
    """Interactive mode with auto-detection"""
    print(f"{'='*60}")
    print("  MakcuFlasher - Interactive Mode")
    print(f"{'='*60}\n")

    # Find serial ports
    ports = find_serial_ports()

    if not ports:
        print("[ERROR] No serial ports detected!")
        print("Please connect your Makcu device and try again.")
        if sys.platform != 'win32':
            print("\nOn Linux, check: ls /dev/ttyUSB* /dev/ttyACM*")
            print("You may need permissions: sudo usermod -a -G dialout $USER")
        return False

    print("Available serial ports:")
    for i, port in enumerate(ports, 1):
        print(f"  {i}. {port}")

    # Select port
    choice = input(f"\nSelect port (1-{len(ports)}) or enter manually: ").strip()

    try:
        idx = int(choice) - 1
        if 0 <= idx < len(ports):
            serial_port = ports[idx]
        else:
            serial_port = choice
    except ValueError:
        serial_port = choice

    # Find firmware files
    firmwares = find_firmware_files()

    if not firmwares:
        print("\n[ERROR] No firmware files found!")
        print("Please place .bin files in the 'firmware' directory.")
        return False

    print("\nAvailable firmware files:")
    for i, fw in enumerate(firmwares, 1):
        print(f"  {i}. {fw}")

    # Select firmware
    choice = input(f"\nSelect firmware (1-{len(firmwares)}) or enter path: ").strip()

    try:
        idx = int(choice) - 1
        if 0 <= idx < len(firmwares):
            firmware_file = firmwares[idx]
        else:
            firmware_file = choice
    except ValueError:
        firmware_file = choice

    print()
    return flash_firmware(serial_port, firmware_file)

def main():
    if len(sys.argv) == 1:
        # Interactive mode
        success = interactive_mode()
        sys.exit(0 if success else 1)

    elif len(sys.argv) == 3:
        # Manual mode: port and firmware
        serial_port = sys.argv[1]
        firmware_file = sys.argv[2]
        success = flash_firmware(serial_port, firmware_file)
        sys.exit(0 if success else 1)

    else:
        print("MakcuFlasher - ESP32-based Makcu firmware uploader\n")
        print("Usage:")
        print("  Interactive mode: python makcu_flash.py")
        print("  Manual mode:      python makcu_flash.py <PORT> <FIRMWARE>\n")
        print("Examples:")
        if sys.platform == 'win32':
            print("  python makcu_flash.py COM3 firmware/V3.8.bin")
        else:
            print("  python makcu_flash.py /dev/ttyACM0 firmware/V3.8.bin")
        sys.exit(1)

if __name__ == '__main__':
    main()
