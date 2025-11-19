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

def find_firmware_versions():
    """Find available firmware versions"""
    search_paths = ['firmware', '../firmware', '.']
    files = []
    
    for path in search_paths:
        pattern = os.path.join(path, '*.bin')
        found = glob.glob(pattern)
        if found:
            files.extend(found)
            break
    
    # Extract unique versions (e.g., V3.8 from V3.8_LEFT.bin)
    versions = set()
    version_map = {} # version -> list of files
    
    for f in files:
        filename = os.path.basename(f)
        # Simple heuristic: take the first part before _ or .bin
        # e.g. V3.8_LEFT.bin -> V3.8
        # e.g. V2.0.bin -> V2.0
        
        base = filename.split('_')[0].replace('.bin', '')
        if base.startswith('V'):
            versions.add(base)
            if base not in version_map:
                version_map[base] = []
            version_map[base].append(f)
            
    sorted_versions = sorted(list(versions))
    return sorted_versions, version_map

def get_firmware_for_target(version, target_type, version_map):
    """Select specific firmware file based on target"""
    files = version_map.get(version, [])
    
    # Target type: 'main' (Flash 1/Left) or 'sub' (Flash 3/Right)
    keyword = 'LEFT' if target_type == 'main' else 'RIGHT'
    
    # 1. Look for exact match with keyword
    for f in files:
        if keyword in os.path.basename(f).upper():
            return f
            
    # 2. Fallback: Look for generic file (no LEFT/RIGHT)
    for f in files:
        upper_name = os.path.basename(f).upper()
        if 'LEFT' not in upper_name and 'RIGHT' not in upper_name:
            return f
            
    # 3. Last resort: return first file
    return files[0] if files else None

def flash_firmware(port, firmware_file, chip='esp32', target_name=None, target_instruction=None):
    """Flash firmware using esptool"""
    print(f"\n{'='*60}")
    print(f"  MakcuFlasher - ESP32 Firmware Uploader")
    print(f"{'='*60}")
    print(f"Target:         {target_name if target_name else 'Generic'}")
    print(f"Serial Port:    {port}")
    print(f"Firmware File:  {firmware_file}")
    print(f"Chip Type:      {chip}")
    print(f"{'='*60}\n")

    if target_instruction:
        print(f"IMPORTANT INSTRUCTION:")
        print(f"  {target_instruction}")
        print(f"{'-'*60}\n")

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
            # Capture output to check for specific errors
            process = subprocess.run(cmd, check=True, capture_output=True, text=True)
            print(process.stdout)
            print(f"\n{'='*60}")
            print("  Firmware upload successful!")
            print(f"{'='*60}")
            return True
        except FileNotFoundError:
            continue
        except subprocess.CalledProcessError as e:
            print(e.stdout)
            print(e.stderr)
            
            if "Permission denied" in e.stderr or "Permission denied" in e.stdout:
                print(f"\n{'='*60}")
                print("  [ERROR] PERMISSION DENIED")
                print(f"{'='*60}")
                print("  You do not have permission to access the serial port.")
                print("  Please run the following command to fix it:")
                print(f"\n    sudo usermod -a -G dialout $USER")
                print("\n  Then LOG OUT and LOG BACK IN for changes to take effect.")
                print(f"{'='*60}")
            else:
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

    # Find firmware versions
    versions, version_map = find_firmware_versions()

    if not versions:
        print("\n[ERROR] No firmware files found!")
        print("Please place .bin files in the 'firmware' directory.")
        return False

    print("\nAvailable Firmware Versions:")
    for i, ver in enumerate(versions, 1):
        print(f"  {i}. {ver}")

    # Select firmware version
    choice = input(f"\nSelect version (1-{len(versions)}): ").strip()

    try:
        idx = int(choice) - 1
        if 0 <= idx < len(versions):
            selected_version = versions[idx]
        else:
            print("Invalid selection.")
            return False
    except ValueError:
        print("Invalid selection.")
        return False

    # Select chip
    print("\nSelect Chip Type:")
    
    # Try to auto-detect chip
    detected_chip = None
    print("  [INFO] Attempting to auto-detect chip type...")
    try:
        # Try to run esptool chip_id command
        cmd = [sys.executable, '-m', 'esptool', '--port', serial_port, 'chip_id']
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=5)
        output = result.stdout
        
        if "ESP32-S3" in output:
            detected_chip = 'esp32s3'
            print("  [SUCCESS] Detected: ESP32-S3")
        elif "ESP32-C3" in output:
            detected_chip = 'esp32c3'
            print("  [SUCCESS] Detected: ESP32-C3")
        elif "ESP32-S2" in output:
            detected_chip = 'esp32s2'
            print("  [SUCCESS] Detected: ESP32-S2")
        elif "ESP32" in output:
            detected_chip = 'esp32'
            print("  [SUCCESS] Detected: ESP32")
        else:
            print("  [WARNING] Could not identify chip type from output.")
    except Exception as e:
        print(f"  [WARNING] Auto-detection failed: {e}")

    print("\n  1. ESP32")
    print("  2. ESP32-S3")
    print("  3. ESP32-C3")
    print("  4. ESP32-S2")

    default_idx = '1'
    if detected_chip == 'esp32s3': default_idx = '2'
    elif detected_chip == 'esp32c3': default_idx = '3'
    elif detected_chip == 'esp32s2': default_idx = '4'

    chip_choice = input(f"\nSelect chip (1-4) [{default_idx}]: ").strip()
    if not chip_choice:
        chip_choice = default_idx

    chips = {
        '1': 'esp32',
        '2': 'esp32s3',
        '3': 'esp32c3',
        '4': 'esp32s2'
    }

    selected_chip = chips.get(chip_choice, 'esp32')

    # Select Flash Target (Main/Sub)
    print("\nSelect Flash Target:")
    print("  1. Flash 1 (Main PC) - USB 1 (Left Port)")
    print("  2. Flash 3 (Sub PC)  - USB 3 (Right Port)")
    
    target_choice = input("\nSelect target (1-2) [1]: ").strip()
    
    if target_choice == '2':
        target_type = 'sub'
        target_name = "Flash 3 (Sub PC)"
        target_instruction = "Connect to USB 3 (Right Port) while holding the RIGHT button"
    else:
        target_type = 'main'
        target_name = "Flash 1 (Main PC)"
        target_instruction = "Connect to USB 1 (Left Port) while holding the LEFT button"

    # Resolve firmware file based on version and target
    firmware_file = get_firmware_for_target(selected_version, target_type, version_map)
    
    if not firmware_file:
        print(f"\n[ERROR] Could not find firmware file for version {selected_version} and target {target_name}")
        return False

    print()
    return flash_firmware(serial_port, firmware_file, chip=selected_chip, target_name=target_name, target_instruction=target_instruction)

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
    
    elif len(sys.argv) == 4:
        # Manual mode with chip: port, firmware, chip
        serial_port = sys.argv[1]
        firmware_file = sys.argv[2]
        chip = sys.argv[3]
        success = flash_firmware(serial_port, firmware_file, chip=chip)
        sys.exit(0 if success else 1)

    else:
        print("MakcuFlasher - ESP32-based Makcu firmware uploader\n")
        print("Usage:")
        print("  Interactive mode: python makcu_flash.py")
        print("  Manual mode:      python makcu_flash.py <PORT> <FIRMWARE> [CHIP]\n")
        print("Examples:")
        if sys.platform == 'win32':
            print("  python makcu_flash.py COM3 firmware/V3.8.bin")
            print("  python makcu_flash.py COM3 firmware/V3.8.bin esp32s3")
        else:
            print("  python makcu_flash.py /dev/ttyACM0 firmware/V3.8.bin")
        sys.exit(1)

if __name__ == '__main__':
    main()
