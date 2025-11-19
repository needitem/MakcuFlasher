#include "MakcuFlasher.h"
#include <iostream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#endif

// Bootloader protocol commands
static const uint8_t CMD_ENTER_BOOTLOADER = 0xA5;
static const uint8_t CMD_ERASE = 0xE0;
static const uint8_t CMD_WRITE = 0xD0;
static const uint8_t CMD_VERIFY = 0xC0;
static const uint8_t CMD_EXIT_BOOTLOADER = 0xF0;
static const uint8_t ACK = 0x79;
static const uint8_t NACK = 0x1F;

static const size_t PAGE_SIZE = 128;
static const int BOOTLOADER_BAUD = 115200;

MakcuFlasher::MakcuFlasher(const std::string& port)
    : is_open_(false), port_name_(port)
{
#ifdef _WIN32
    serial_handle_ = INVALID_HANDLE_VALUE;
#else
    serial_fd_ = -1;
#endif
    openPort(port);
}

MakcuFlasher::~MakcuFlasher()
{
    closePort();
}

bool MakcuFlasher::isOpen() const
{
    return is_open_;
}

bool MakcuFlasher::openPort(const std::string& port)
{
#ifdef _WIN32
    serial_handle_ = CreateFileA(
        port.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (serial_handle_ == INVALID_HANDLE_VALUE) {
        std::cerr << "[MakcuFlasher] Failed to open port " << port << std::endl;
        return false;
    }

    DCB dcb = {};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(serial_handle_, &dcb)) {
        std::cerr << "[MakcuFlasher] GetCommState failed" << std::endl;
        CloseHandle(serial_handle_);
        serial_handle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    dcb.BaudRate = BOOTLOADER_BAUD;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(serial_handle_, &dcb)) {
        std::cerr << "[MakcuFlasher] SetCommState failed" << std::endl;
        CloseHandle(serial_handle_);
        serial_handle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    COMMTIMEOUTS timeouts = {};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 500;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(serial_handle_, &timeouts)) {
        std::cerr << "[MakcuFlasher] SetCommTimeouts failed" << std::endl;
        CloseHandle(serial_handle_);
        serial_handle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    is_open_ = true;
    std::cout << "[MakcuFlasher] Opened " << port << " at " << BOOTLOADER_BAUD << " baud (Windows API)" << std::endl;

#else // Linux
    serial_fd_ = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd_ < 0) {
        std::cerr << "[MakcuFlasher] Failed to open port " << port << ": " << strerror(errno) << std::endl;
        return false;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(serial_fd_, &tty) != 0) {
        std::cerr << "[MakcuFlasher] tcgetattr failed: " << strerror(errno) << std::endl;
        close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 10;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(serial_fd_, TCSANOW, &tty) != 0) {
        std::cerr << "[MakcuFlasher] tcsetattr failed: " << strerror(errno) << std::endl;
        close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    is_open_ = true;
    std::cout << "[MakcuFlasher] Opened " << port << " at " << BOOTLOADER_BAUD << " baud (Linux termios)" << std::endl;
#endif

    return true;
}

void MakcuFlasher::closePort()
{
    if (!is_open_) {
        return;
    }

#ifdef _WIN32
    if (serial_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(serial_handle_);
        serial_handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (serial_fd_ >= 0) {
        close(serial_fd_);
        serial_fd_ = -1;
    }
#endif

    is_open_ = false;
}

bool MakcuFlasher::writeBytes(const uint8_t* data, size_t length)
{
    if (!is_open_) {
        return false;
    }

#ifdef _WIN32
    DWORD written = 0;
    if (!WriteFile(serial_handle_, data, static_cast<DWORD>(length), &written, nullptr)) {
        return false;
    }
    return written == length;
#else
    ssize_t written = write(serial_fd_, data, length);
    return written == static_cast<ssize_t>(length);
#endif
}

bool MakcuFlasher::readBytes(uint8_t* buffer, size_t length, int timeoutMs)
{
    if (!is_open_) {
        return false;
    }

#ifdef _WIN32
    COMMTIMEOUTS timeouts = {};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = timeoutMs;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(serial_handle_, &timeouts);

    DWORD read_count = 0;
    if (!ReadFile(serial_handle_, buffer, static_cast<DWORD>(length), &read_count, nullptr)) {
        return false;
    }
    return read_count == length;
#else
    auto start = std::chrono::steady_clock::now();
    size_t total_read = 0;

    while (total_read < length) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (elapsed_ms >= timeoutMs) {
            return false;
        }

        ssize_t n = read(serial_fd_, buffer + total_read, length - total_read);
        if (n > 0) {
            total_read += n;
        } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            return false;
        }

        if (total_read < length) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    return total_read == length;
#endif
}

bool MakcuFlasher::waitForAck(int timeoutMs)
{
    uint8_t response = 0;
    if (!readBytes(&response, 1, timeoutMs)) {
        std::cerr << "[MakcuFlasher] Timeout waiting for ACK" << std::endl;
        return false;
    }

    if (response == NACK) {
        std::cerr << "[MakcuFlasher] Received NACK" << std::endl;
        return false;
    }

    if (response != ACK) {
        std::cerr << "[MakcuFlasher] Unexpected response: 0x" << std::hex << (int)response << std::dec << std::endl;
        return false;
    }

    return true;
}

bool MakcuFlasher::enterBootloaderMode()
{
    std::cout << "[MakcuFlasher] Entering bootloader mode..." << std::endl;

    uint8_t cmd = CMD_ENTER_BOOTLOADER;
    if (!writeBytes(&cmd, 1)) {
        std::cerr << "[MakcuFlasher] Failed to send enter bootloader command" << std::endl;
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (!waitForAck(2000)) {
        std::cerr << "[MakcuFlasher] Device did not acknowledge bootloader entry" << std::endl;
        return false;
    }

    std::cout << "[MakcuFlasher] Successfully entered bootloader mode" << std::endl;
    return true;
}

bool MakcuFlasher::eraseFirmware()
{
    std::cout << "[MakcuFlasher] Erasing flash memory..." << std::endl;

    uint8_t cmd = CMD_ERASE;
    if (!writeBytes(&cmd, 1)) {
        std::cerr << "[MakcuFlasher] Failed to send erase command" << std::endl;
        return false;
    }

    if (!waitForAck(5000)) {
        std::cerr << "[MakcuFlasher] Flash erase failed" << std::endl;
        return false;
    }

    std::cout << "[MakcuFlasher] Flash erased successfully" << std::endl;
    return true;
}

bool MakcuFlasher::writeFirmwarePage(uint32_t address, const uint8_t* data, size_t length)
{
    if (length > PAGE_SIZE) {
        return false;
    }

    uint8_t cmd_packet[5 + PAGE_SIZE];
    cmd_packet[0] = CMD_WRITE;
    cmd_packet[1] = (address >> 24) & 0xFF;
    cmd_packet[2] = (address >> 16) & 0xFF;
    cmd_packet[3] = (address >> 8) & 0xFF;
    cmd_packet[4] = address & 0xFF;

    memcpy(&cmd_packet[5], data, length);
    if (length < PAGE_SIZE) {
        memset(&cmd_packet[5 + length], 0xFF, PAGE_SIZE - length);
    }

    if (!writeBytes(cmd_packet, 5 + PAGE_SIZE)) {
        return false;
    }

    return waitForAck(1000);
}

bool MakcuFlasher::verifyFirmware(const std::vector<uint8_t>& firmware)
{
    std::cout << "[MakcuFlasher] Verifying firmware..." << std::endl;

    uint8_t cmd = CMD_VERIFY;
    if (!writeBytes(&cmd, 1)) {
        return false;
    }

    uint32_t checksum = 0;
    for (uint8_t byte : firmware) {
        checksum += byte;
    }

    uint8_t checksum_packet[4];
    checksum_packet[0] = (checksum >> 24) & 0xFF;
    checksum_packet[1] = (checksum >> 16) & 0xFF;
    checksum_packet[2] = (checksum >> 8) & 0xFF;
    checksum_packet[3] = checksum & 0xFF;

    if (!writeBytes(checksum_packet, 4)) {
        return false;
    }

    if (!waitForAck(3000)) {
        std::cerr << "[MakcuFlasher] Firmware verification failed" << std::endl;
        return false;
    }

    std::cout << "[MakcuFlasher] Firmware verified successfully" << std::endl;
    return true;
}

bool MakcuFlasher::exitBootloaderMode()
{
    std::cout << "[MakcuFlasher] Exiting bootloader mode..." << std::endl;

    uint8_t cmd = CMD_EXIT_BOOTLOADER;
    if (!writeBytes(&cmd, 1)) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "[MakcuFlasher] Device reset, bootloader exited" << std::endl;
    return true;
}

bool MakcuFlasher::uploadFirmware(const std::vector<uint8_t>& firmware)
{
    if (firmware.empty()) {
        std::cerr << "[MakcuFlasher] Firmware data is empty" << std::endl;
        return false;
    }

    std::cout << "[MakcuFlasher] Starting firmware upload (" << firmware.size() << " bytes)..." << std::endl;

    if (!enterBootloaderMode()) {
        return false;
    }

    if (!eraseFirmware()) {
        return false;
    }

    size_t total_pages = (firmware.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    std::cout << "[MakcuFlasher] Writing " << total_pages << " pages..." << std::endl;

    for (size_t i = 0; i < total_pages; i++) {
        uint32_t address = i * PAGE_SIZE;
        size_t remaining = firmware.size() - address;
        size_t page_length = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;

        std::cout << "\r[MakcuFlasher] Progress: " << (i + 1) << "/" << total_pages << " pages" << std::flush;

        if (!writeFirmwarePage(address, &firmware[address], page_length)) {
            std::cerr << "\n[MakcuFlasher] Failed to write page " << i << std::endl;
            return false;
        }
    }

    std::cout << std::endl;

    if (!verifyFirmware(firmware)) {
        return false;
    }

    if (!exitBootloaderMode()) {
        return false;
    }

    std::cout << "[MakcuFlasher] Firmware upload complete!" << std::endl;
    return true;
}
