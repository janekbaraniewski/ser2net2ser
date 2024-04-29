#include "VirtualSerialPort.h"

VirtualSerialPort::VirtualSerialPort(const std::string& device) : device_name_("/dev/" + device) {
    master_fd_ = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd_ == -1 || grantpt(master_fd_) != 0 || unlockpt(master_fd_) != 0) {
        throw std::runtime_error("Failed to open PTY master");
    }

    char* slave_name = ptsname(master_fd_);
    if (!slave_name) {
        throw std::runtime_error("Failed to get PTY slave name");
    }

    if (unlink(device_name_.c_str()) == -1 && errno != ENOENT) {
        throw std::runtime_error("Failed to remove existing symlink");
    }

    if (symlink(slave_name, device_name_.c_str()) != 0) {
        throw std::runtime_error("Failed to create symlink for PTY slave");
    }

    chmod(device_name_.c_str(), 0660);
    chown(device_name_.c_str(), -1, getgrnam("tty")->gr_gid);  // Set group to 'tty'

    slave_fd_ = ::open(slave_name, O_RDWR);
    if (slave_fd_ == -1) {
        throw std::runtime_error("Failed to open PTY slave");
    }

    // Configure terminal settings
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(slave_fd_, &tty) != 0) {
        throw std::runtime_error("Failed to get terminal attributes");
    }

    cfmakeraw(&tty);  // Make the terminal raw
    tty.c_cflag |= CREAD | CLOCAL;  // Enable receiver, Ignore modem control lines
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;  // 8-bit characters
    tty.c_cflag &= ~PARENB;  // No parity bit
    tty.c_cflag &= ~CSTOPB;  // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;  // No hardware flow control
    cfsetispeed(&tty, B115200);  // Input speed
    cfsetospeed(&tty, B115200);  // Output speed

    if (tcsetattr(slave_fd_, TCSANOW, &tty) != 0) {
        throw std::runtime_error("Failed to set terminal attributes");
    }
}

VirtualSerialPort::~VirtualSerialPort() {
    close();
}

void VirtualSerialPort::close() {
    if (master_fd_ != -1) {
        ::close(master_fd_);
        master_fd_ = -1;
    }
    if (slave_fd_ != -1) {
        ::close(slave_fd_);
        slave_fd_ = -1;
    }
    unlink(device_name_.c_str());
}

bool VirtualSerialPort::write(const std::string& data) {
    if (master_fd_ == -1) {
        BOOST_LOG_TRIVIAL(error) << "Attempt to write with closed master FD";
        return false;
    }

    ssize_t written = 0;
    ssize_t to_write = data.size();
    while (written < to_write) {
        ssize_t result = ::write(master_fd_, data.c_str() + written, to_write - written);
        if (result < 0) {
            BOOST_LOG_TRIVIAL(error) << "Write error: " << strerror(errno);
            return false;
        }
        written += result;
    }
    return true;
}

std::string VirtualSerialPort::read() {
    if (master_fd_ == -1) {
        BOOST_LOG_TRIVIAL(error) << "Attempt to read with closed master FD";
        return "";
    }

    char buffer[256];
    ssize_t len = ::read(master_fd_, buffer, sizeof(buffer) - 1);
    if (len < 0) {
        BOOST_LOG_TRIVIAL(error) << "Read error: " << strerror(errno);
        return "";
    } else if (len == 0) {
        BOOST_LOG_TRIVIAL(info) << "No data read";
        return "";
    }

    buffer[len] = '\0';
    return std::string(buffer, len);
}
