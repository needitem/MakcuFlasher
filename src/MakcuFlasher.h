#ifndef MAKCUFLASHER_H
#define MAKCUFLASHER_H

#include <cstdint>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class MakcuFlasher {
public:
    MakcuFlasher(const std::string& port);
    ~MakcuFlasher();

    bool isOpen() const;
    bool uploadFirmware(const std::vector<uint8_t>& firmware);

private:
    bool openPort(const std::string& port);
    void closePort();
    bool enterBootloaderMode();
    bool eraseFirmware();
    bool writeFirmwarePage(uint32_t address, const uint8_t* data, size_t length);
    bool verifyFirmware(const std::vector<uint8_t>& firmware);
    bool exitBootloaderMode();

    bool writeBytes(const uint8_t* data, size_t length);
    bool readBytes(uint8_t* buffer, size_t length, int timeoutMs);
    bool waitForAck(int timeoutMs = 1000);

#ifdef _WIN32
    HANDLE serial_handle_;
#else
    int serial_fd_;
#endif

    bool is_open_;
    std::string port_name_;
};

#endif // MAKCUFLASHER_H
