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
