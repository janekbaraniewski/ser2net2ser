#ifndef ISERIALPORT_H
#define ISERIALPORT_H

#include <string>
#include <functional>

using std::string;

class ISerialPort {
public:
    virtual void open(const string& device, unsigned int baudRate) = 0;
    virtual ssize_t async_read_some(char* buffer, size_t size) = 0;
    virtual ssize_t async_write(const char* buffer, size_t size) = 0;
    virtual ~ISerialPort() = default;
};

#endif // ISERIALPORT_H
