#include "SerialPort.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <stdexcept>

SerialPort::SerialPort(const std::string& device, int baud_rate) {
    serial_fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0) {
        throw std::runtime_error("Error opening serial port");
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(serial_fd, &tty) != 0) {
        throw std::runtime_error("Error from tcgetattr");
    }

    cfsetospeed(&tty, baud_rate);
    cfsetispeed(&tty, baud_rate);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        throw std::runtime_error("Error from tcsetattr");
    }
}

SerialPort::~SerialPort() {
    close(serial_fd);
}

void SerialPort::writeData(const std::string& data) {
    write(serial_fd, data.c_str(), data.size());
}

std::string SerialPort::readData() {
    char buf[256];
    int n = read(serial_fd, buf, sizeof(buf));
    return std::string(buf, n);
}
