#ifndef REALSERIALPORT_H
#define REALSERIALPORT_H

#include "ISerialPort.h"
#include "Logger.h"
#include <string>
#include <functional>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>

class RealSerialPort : public ISerialPort {
public:
    explicit RealSerialPort(const std::string& device, unsigned int baudRate);
    ~RealSerialPort();

    void open(const std::string& device, unsigned int baudRate) override;
    ssize_t async_read_some(char* buffer, size_t size) override;
    ssize_t async_write(const char* buffer, size_t size) override;

private:
    int fd;  // File descriptor for the serial port
    void configurePort(unsigned int baudRate);
};

#endif // REALSERIALPORT_H
