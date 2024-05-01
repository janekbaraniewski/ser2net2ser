#ifndef VIRTUALSERIALPORT_H
#define VIRTUALSERIALPORT_H

#include "common.hpp"
#include <grp.h>

class VirtualSerialPort {
public:
    VirtualSerialPort(boost::asio::io_context& io_context, const std::string& device);
    ~VirtualSerialPort();

    void async_read(boost::asio::mutable_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void async_write(boost::asio::const_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void close();

private:
    boost::asio::posix::stream_descriptor master_fd_;
    boost::asio::posix::stream_descriptor slave_fd_;
    boost::array<char, 128> buffer_;
    // std::array<char, 1024> read_buffer_;
    std::string device_name_;
    std::mutex mutex_;

    void setup_pty(int fd);
};

#endif // VIRTUALSERIALPORT_H
