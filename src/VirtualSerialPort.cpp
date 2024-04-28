#include "virtualserialport.h"

VirtualSerialPort::VirtualSerialPort(const std::string& device) : device_name_("/dev/" + device) {
    master_fd_ = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd_ == -1 || grantpt(master_fd_) != 0 || unlockpt(master_fd_) != 0) {
        throw std::runtime_error("Failed to open PTY master");
    }

    char* slave_name = ptsname(master_fd_);
    if (!slave_name) {
        throw std::runtime_error("Failed to get PTY slave name");
    }

    unlink(device_name_.c_str());  // Ensure no stale symlink exists
    if (symlink(slave_name, device_name_.c_str()) != 0) {
        throw std::runtime_error("Failed to create symlink for PTY slave");
    }

    slave_fd_ = ::open(slave_name, O_RDWR);
    if (slave_fd_ == -1) {
        throw std::runtime_error("Failed to open PTY slave");
    }

    // Set permissions to allow external applications to access the virtual serial port
    chmod(device_name_.c_str(), 0666);
}

VirtualSerialPort::~VirtualSerialPort() {
    close();
}

void VirtualSerialPort::close() {
    ::close(master_fd_);
    ::close(slave_fd_);
    unlink(device_name_.c_str());  // Clean up the symlink on destruction
}

bool VirtualSerialPort::write(const std::string& data) {
    ssize_t size = ::write(master_fd_, data.c_str(), data.size());
    return size >= 0;
}

std::string VirtualSerialPort::read() {
    char buffer[256];
    ssize_t len = ::read(master_fd_, buffer, sizeof(buffer) - 1);
    if (len > 0) {
        buffer[len] = '\0';
        return std::string(buffer);
    }
    return "";
}
