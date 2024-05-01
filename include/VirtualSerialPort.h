#ifndef VIRTUALSERIALPORT_H
#define VIRTUALSERIALPORT_H

#include "common.hpp"
#include <grp.h>
#include <sys/stat.h>

class VirtualSerialPort {
public:
    int master_fd_raw_;

    VirtualSerialPort(const std::string& device);
    ~VirtualSerialPort();

    ssize_t async_read(char* buffer, unsigned int length);
    ssize_t async_write(const char* buffer, unsigned int length);

private:
    int slave_fd_raw_;
    std::string device_name_;
    std::mutex mutex_;

    void setup_pty(int fd);
};

#endif // VIRTUALSERIALPORT_H
