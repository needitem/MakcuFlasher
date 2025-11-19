#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "MakcuFlasher.h"

std::vector<std::string> findSerialPorts()
{
    std::vector<std::string> ports;

#ifdef _WIN32
    for (int i = 1; i <= 256; i++) {
        std::string port = "COM" + std::to_string(i);
        std::string full_port = "\\\\.\\" + port;

        HANDLE h = CreateFileA(full_port.c_str(), GENERIC_READ | GENERIC_WRITE,
                               0, nullptr, OPEN_EXISTING, 0, nullptr);

        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            ports.push_back(port);
        }
    }
#else
    DIR* dir = opendir("/dev");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find("ttyUSB") == 0 || name.find("ttyACM") == 0) {
                ports.push_back("/dev/" + name);
            }
        }
        closedir(dir);
    }
    std::sort(ports.begin(), ports.end());
#endif

    return ports;
}

std::vector<std::string> findFirmwareFiles(const std::string& base_path = ".")
{
    std::vector<std::string> files;
    std::vector<std::string> search_paths = {
        base_path + "/firmware",
        base_path + "/../firmware",
        base_path,
        "firmware",
        "."
    };

    for (const auto& path : search_paths) {
#ifdef _WIN32
        WIN32_FIND_DATAA find_data;
        HANDLE hFind = FindFirstFileA((path + "/*.bin").c_str(), &find_data);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::string filename = find_data.cFileName;
                if (filename.find("V") == 0 || filename.find("v") == 0) {
                    files.push_back(path + "/" + filename);
                }
            } while (FindNextFileA(hFind, &find_data) != 0);
            FindClose(hFind);

            if (!files.empty()) break;
        }
#else
        DIR* dir = opendir(path.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if ((name.find("V") == 0 || name.find("v") == 0) &&
                    name.find(".bin") != std::string::npos) {
                    files.push_back(path + "/" + name);
                }
            }
            closedir(dir);

            if (!files.empty()) break;
        }
#endif
    }

    std::sort(files.begin(), files.end());
    return files;
}

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

std::string getUserInput(const std::string& prompt)
{
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

void printUsage(const char* program_name)
{
    std::cout << "MakcuFlasher - Cross-platform Makcu firmware uploader\n\n";
    std::cout << "Usage: " << program_name << " [SERIAL_PORT] [FIRMWARE_FILE]\n\n";
    std::cout << "Interactive mode (no arguments):\n";
    std::cout << "  " << program_name << "\n\n";
    std::cout << "Manual mode:\n";
#ifdef _WIN32
    std::cout << "  " << program_name << " COM3 firmware/V3.8.bin\n";
    std::cout << "  " << program_name << " COM4 V3.8.bin\n";
#else
    std::cout << "  " << program_name << " /dev/ttyUSB0 firmware/V3.8.bin\n";
    std::cout << "  " << program_name << " /dev/ttyACM0 V3.8.bin\n";
#endif
    std::cout << "\nSupported firmware versions: V2.0, V3.0, V3.2, V3.4, V3.7, V3.8\n";
}

int main(int argc, char** argv)
{
    std::string serial_port;
    std::string firmware_file;

    if (argc == 1) {
        std::cout << "==================================================\n";
        std::cout << "    MakcuFlasher - Interactive Mode\n";
        std::cout << "==================================================\n\n";

        auto ports = findSerialPorts();

        if (ports.empty()) {
            std::cerr << "[MakcuFlasher] No serial ports detected!\n";
            std::cerr << "Please connect your Makcu device and try again.\n";
#ifndef _WIN32
            std::cerr << "\nOn Linux, check: ls /dev/ttyUSB* /dev/ttyACM*\n";
            std::cerr << "You may need permissions: sudo usermod -a -G dialout $USER\n";
#endif
            return 1;
        }

        std::cout << "Available serial ports:\n";
        for (size_t i = 0; i < ports.size(); i++) {
            std::cout << "  " << (i + 1) << ". " << ports[i] << "\n";
        }

        std::string choice = getUserInput("\nSelect port (1-" + std::to_string(ports.size()) + ") or enter manually: ");

        try {
            int idx = std::stoi(choice) - 1;
            if (idx >= 0 && idx < static_cast<int>(ports.size())) {
                serial_port = ports[idx];
            } else {
                serial_port = choice;
            }
        } catch (...) {
            serial_port = choice;
        }

        auto firmwares = findFirmwareFiles();

        if (firmwares.empty()) {
            std::cerr << "\n[MakcuFlasher] No firmware files found!\n";
            std::cerr << "Please place .bin files in the 'firmware' directory.\n";
            std::cerr << "Download from: https://github.com/terrafirma2021/MAKCM_v2_files\n";
            return 1;
        }

        std::cout << "\nAvailable firmware files:\n";
        for (size_t i = 0; i < firmwares.size(); i++) {
            std::cout << "  " << (i + 1) << ". " << firmwares[i] << "\n";
        }

        choice = getUserInput("\nSelect firmware (1-" + std::to_string(firmwares.size()) + ") or enter path: ");

        try {
            int idx = std::stoi(choice) - 1;
            if (idx >= 0 && idx < static_cast<int>(firmwares.size())) {
                firmware_file = firmwares[idx];
            } else {
                firmware_file = choice;
            }
        } catch (...) {
            firmware_file = choice;
        }

        std::cout << "\n";

    } else if (argc == 3) {
        serial_port = argv[1];
        firmware_file = argv[2];
    } else {
        printUsage(argv[0]);
        return 1;
    }

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
