#ifndef VIRTUALSERIALPORT_H
#define VIRTUALSERIALPORT_H

#include "common.hpp"
#include <grp.h>

class VirtualSerialPort {
public:
    int master_fd_raw_;

    VirtualSerialPort(boost::asio::io_context& io_context, const std::string& device);
    ~VirtualSerialPort();

    ssize_t async_read(char* buffer, unsigned int length);
    ssize_t async_write(const char* buffer, unsigned int length);

private:
    int slave_fd_raw_;
    // boost::array<char, 128> buffer_;
    // std::array<char, 1024> read_buffer_;
    std::string device_name_;
    std::mutex mutex_;

    void setup_pty(int fd);
};

#endif // VIRTUALSERIALPORT_H
