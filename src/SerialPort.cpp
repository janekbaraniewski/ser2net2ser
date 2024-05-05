#include "SerialPort.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <stdexcept>

SerialPort::SerialPort(const std::string& device, int baud_rate) {
    Logger(Logger::Level::Info) << "SerialPort init start - device " << device << " - baudRate - " << baud_rate;
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
    Logger(Logger::Level::Info) << "SerialPort init finish";
}

SerialPort::~SerialPort() {
    close(serial_fd);
}

void SerialPort::writeData(const std::string& data) {
    Logger(Logger::Level::Info) << "SerialPort write data - " << data;
    write(serial_fd, data.c_str(), data.size());
}


void SerialPort::readLoop() {
    Logger(Logger::Level::Info) << "SerialPort start reading loop";
    while (keep_reading) {
        char buf[256];
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
