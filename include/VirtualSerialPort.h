#ifndef VIRTUALSERIALPORT_H
#define VIRTUALSERIALPORT_H

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
    VirtualSerialPort(const std::string& device);
    ~VirtualSerialPort();

    void close();
    bool write(const std::string& data);
    std::string read();

private:
    int master_fd_;
    int slave_fd_;
    std::string device_name_;
};

#endif // VIRTUALSERIALPORT_H
