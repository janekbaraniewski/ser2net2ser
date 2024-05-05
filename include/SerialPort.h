#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <string>

class SerialPort {
private:
    int serial_fd;

public:
    SerialPort(const std::string& device, int baud_rate);
    ~SerialPort();
    void writeData(const std::string& data);
    std::string readData();
};

#endif // SERIALPORT_H
