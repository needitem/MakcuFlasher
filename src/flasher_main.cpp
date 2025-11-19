#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "MakcuFlasher.h"

std::vector<uint8_t> readFirmwareFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "[MakcuFlasher] Failed to open firmware file: " << filename << std::endl;
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "[MakcuFlasher] Failed to read firmware file" << std::endl;
        return {};
    }

    return buffer;
}

void printUsage(const char* program_name)
{
    std::cout << "MakcuFlasher - Cross-platform Makcu firmware uploader\n\n";
    std::cout << "Usage: " << program_name << " <SERIAL_PORT> <FIRMWARE_FILE>\n\n";
    std::cout << "Examples:\n";
#ifdef _WIN32
    std::cout << "  " << program_name << " COM3 firmware_v3.8.bin\n";
    std::cout << "  " << program_name << " COM4 V3.8.bin\n";
#else
    std::cout << "  " << program_name << " /dev/ttyUSB0 firmware_v3.8.bin\n";
    std::cout << "  " << program_name << " /dev/ttyACM0 V3.8.bin\n";
#endif
    std::cout << "\nSupported firmware versions: V2.0, V3.0, V3.2, V3.4, V3.7, V3.8\n";
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    std::string serial_port = argv[1];
    std::string firmware_file = argv[2];

    std::cout << "==================================================\n";
    std::cout << "          MakcuFlasher - Firmware Uploader        \n";
    std::cout << "==================================================\n";
    std::cout << "Serial Port:    " << serial_port << "\n";
    std::cout << "Firmware File:  " << firmware_file << "\n";
    std::cout << "==================================================\n\n";

    auto firmware = readFirmwareFile(firmware_file);
    if (firmware.empty()) {
        return 1;
    }

    std::cout << "[MakcuFlasher] Loaded " << firmware.size() << " bytes from " << firmware_file << "\n\n";

    MakcuFlasher flasher(serial_port);
    if (!flasher.isOpen()) {
        std::cerr << "[MakcuFlasher] Failed to open serial port " << serial_port << std::endl;
#ifndef _WIN32
        std::cerr << "[MakcuFlasher] On Linux, you may need to:\n";
        std::cerr << "  1. Add your user to the dialout group: sudo usermod -a -G dialout $USER\n";
        std::cerr << "  2. Log out and log back in\n";
        std::cerr << "  3. Or run with sudo (not recommended)\n";
#endif
        return 1;
    }

    std::cout << "\n**************************************************\n";
    std::cout << "  WARNING: Do not disconnect the device during\n";
    std::cout << "  the firmware upload process!\n";
    std::cout << "**************************************************\n\n";

    bool success = flasher.uploadFirmware(firmware);

    std::cout << "\n==================================================\n";
    if (success) {
        std::cout << "  Firmware upload successful!\n";
        std::cout << "==================================================\n";
        return 0;
    } else {
        std::cout << "  Firmware upload failed!\n";
        std::cout << "==================================================\n";
        return 1;
    }
}
