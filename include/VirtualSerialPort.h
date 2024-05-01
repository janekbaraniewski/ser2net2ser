#ifndef VIRTUALSERIALPORT_H
#define VIRTUALSERIALPORT_H

#include "common.hpp"
#include <grp.h>

class VirtualSerialPort {
public:
    boost::asio::posix::stream_descriptor master_fd_;

    VirtualSerialPort(boost::asio::io_context& io_context, const std::string& device);
    ~VirtualSerialPort();

    ssize_t async_read(char* buffer, unsigned int length);
    ssize_t async_write(const char* buffer, unsigned int length);
    void close();

private:
    boost::asio::posix::stream_descriptor slave_fd_;
    // boost::array<char, 128> buffer_;
    // std::array<char, 1024> read_buffer_;
    std::string device_name_;
    std::mutex mutex_;

    void setup_pty(int fd);
};

#endif // VIRTUALSERIALPORT_H
