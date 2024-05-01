#include "RealSerialPort.h"

RealSerialPort::RealSerialPort(const std::string& device, unsigned int baudRate) : fd(-1) {
    open(device, baudRate);
}

RealSerialPort::~RealSerialPort() {
    if (fd != -1) {
        close(fd);
    }
}

void RealSerialPort::open(const std::string& device, unsigned int baudRate) {
    fd = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1) {
        Logger(Logger::Level::Error) << "Failed to open serial port: " << device << " with error: " << strerror(errno);
        throw std::runtime_error("Failed to open serial port");
    }

    configurePort(baudRate);
    Logger(Logger::Level::Info) << "Serial port opened and configured on " << device;
}

void RealSerialPort::configurePort(unsigned int baudRate) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        Logger(Logger::Level::Error) << "Error from tcgetattr: " << strerror(errno);
        throw std::runtime_error("Error configuring serial port");
    }

    cfsetospeed(&tty, baudRate);
    cfsetispeed(&tty, baudRate);

    tty.c_cflag |= (CLOCAL | CREAD);    // Ignore modem controls, enable reading
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;                 // 8-bit characters
    tty.c_cflag &= ~PARENB;             // No parity bit
    tty.c_cflag &= ~CSTOPB;             // Only need 1 stop bit
    tty.c_cflag &= ~CRTSCTS;            // No hardware flow control

    // Setup for non-canonical mode
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 1;                 // Minimum number of characters to read
    tty.c_cc[VTIME] = 1;                // Time to wait for data (tenths of seconds)

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        Logger(Logger::Level::Error) << "Error from tcsetattr: " << strerror(errno);
        throw std::runtime_error("Error applying serial port settings");
    }
}

ssize_t RealSerialPort::async_read_some(char* buffer, size_t size) {
    ssize_t n = read(fd, buffer, size);
    if (n < 0) {
        Logger(Logger::Level::Error) << "Read error on serial port: " << strerror(errno);
    }
    return n;
}

ssize_t RealSerialPort::async_write(const char* buffer, size_t size) {
    ssize_t n = write(fd, buffer, size);
    if (n < 0) {
        Logger(Logger::Level::Error) << "Write error on serial port: " << strerror(errno);
    }
    return n;
}
