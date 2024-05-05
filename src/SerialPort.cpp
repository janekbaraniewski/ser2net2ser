#include "SerialPort.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <stdexcept>

SerialPort::SerialPort(const std::string& device, int baud_rate) {
    Logger(Logger::Level::Info) << "SerialPort init start - device " << device << " - baudRate - " << baud_rate << std::endl;
    serial_fd = open(device.c_str(), O_RDWR | O_NOCTTY);
    if (serial_fd < 0) {
        Logger(Logger::Level::Error) << "Error opening serial port: " << strerror(errno) << std::endl;
        throw std::runtime_error("Error opening serial port");
    }

    configurePort(baud_rate);
    Logger(Logger::Level::Info) << "SerialPort init finish" << std::endl;
}

SerialPort::~SerialPort() {
    close(serial_fd);
}


void SerialPort::configurePort(int baud_rate) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(serial_fd, &tty) != 0) {
        Logger(Logger::Level::Error) << "tcgetattr failed: " << strerror(errno) << std::endl;
        throw std::runtime_error("tcgetattr failed");
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag |= (CLOCAL | CREAD); // Ignore modem controls, enable reading
    tty.c_cflag &= ~CSIZE; // Clear the mask
    tty.c_cflag |= CS8;    // 8 data bits
    tty.c_cflag &= ~PARENB; // No parity bit
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CRTSCTS; // No hardware flow control

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off software flow control
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Disable canonical mode, echo, and signal chars
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (raw output)

    // Set read conditions: minimal character and timing
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        Logger(Logger::Level::Error) << "tcsetattr failed: " << strerror(errno) << std::endl;
        throw std::runtime_error("tcsetattr failed");
    }
}

void SerialPort::writeData(const std::string& data) {
    Logger(Logger::Level::Info) << "SerialPort write data - " << data;
    write(serial_fd, data.c_str(), data.size());
}


void SerialPort::readLoop() {
    Logger(Logger::Level::Info) << "SerialPort start reading loop";
    while (keep_reading) {
        char buf[1024];
        int n = read(serial_fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';  // Ensure null-termination
            Logger(Logger::Level::Info) << "SerialPort read - " << buf;
            std::lock_guard<std::mutex> lock(mtx);
            read_buffer.push(std::string(buf));
            cv.notify_one();
        }
    }
}

void SerialPort::startReading() {
    keep_reading = true;
    read_thread = std::thread(&SerialPort::readLoop, this);
}

void SerialPort::stopReading() {
    keep_reading = false;
    if (read_thread.joinable())
        read_thread.join();
}

std::string SerialPort::readData() {
    Logger(Logger::Level::Info) << "SerialPort read data";
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]{ return !read_buffer.empty(); });
    std::string data = read_buffer.front();
    read_buffer.pop();
    Logger(Logger::Level::Info) << "SerialPort read data - got data - " << data;
    return data;
}
