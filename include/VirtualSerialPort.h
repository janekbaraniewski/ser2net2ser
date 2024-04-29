#ifndef VIRTUALSERIALPORT_H
#define VIRTUALSERIALPORT_H

#include <boost/asio.hpp>
#include <string>
#include <functional>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdexcept>
#include <iostream>
#include <sys/stat.h>
#include <boost/log/trivial.hpp>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <grp.h>

class VirtualSerialPort {
public:
    VirtualSerialPort(boost::asio::io_context& io_context, const std::string& device);
    ~VirtualSerialPort();

    void close();
    void async_read(boost::asio::mutable_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void async_write(boost::asio::const_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void setSerialAttributes(int fd);
    // bool write(const std::string& data);
    // std::string read();

private:
    boost::asio::posix::stream_descriptor master_fd_;
    std::string device_name_;
};

#endif // VIRTUALSERIALPORT_H
