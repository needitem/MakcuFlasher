
*   **One-Click Flashing**: Simple Python-based interface.
*   **HWID Spoofing**: Randomizes USB descriptors (VID/PID/Serial) to prevent device blacklisting.
*   **Firmware Management**: Automatically manages compatible firmware versions.
*   **Device Detection**: Auto-detects connected ATmega32u4 devices (Leonardo/Micro).

## Supported Hardware

*   **Arduino Leonardo** (ATmega32u4)
*   **Arduino Micro** (ATmega32u4)
*   **Compatible Clones** (Pro Micro, etc.)

## Prerequisites

*   **Python**: 3.10 or higher.
*   **Dependencies**: `pyserial`, `esptool` (if using ESP32), etc.

## Installation

1.  Clone the repository or download the script.
2.  Install dependencies:
    ```bash
    pip install pyserial requests
    ```

## Usage

1.  **Connect Device**: Plug your Arduino device into the PC via USB.
2.  **Run the Script**:
    ```bash
    python makcu_flash.py
    ```
3.  **Select Port**: Follow the on-screen prompts to select your device's COM port.
4.  **Flash**: The script will handle the flashing process.
    *   *Note*: Do not unplug the device during this process.
5.  **Verify**: Once complete, the device will reboot. Note the new COM port assigned to it.

## Integration with NeedAimBot

After flashing:
1.  Open `config.ini` in your **NeedAimBot** folder.
2.  Set `input_method = ARDUINO`.
3.  Set `arduino_port` to the COM port of your flashed device.
4.  Set `arduino_baudrate` to match the firmware (usually `2000000` or `115200`).

## Disclaimer

This software involves modifying hardware firmware. The authors are not responsible for any damage to your devices ("bricking") or any account bans resulting from the use of this hardware in games. Use at your own risk.
