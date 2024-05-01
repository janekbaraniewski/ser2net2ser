#include "VirtualSerialPort.h"

VirtualSerialPort::VirtualSerialPort(const std::string& device)
    : device_name_("/dev/" + device) {
    std::lock_guard<std::mutex> lock(mutex_);
    char* slave_name;
    master_fd_raw_ = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd_raw_ == -1) {
        Logger(Logger::Level::Error) << "Failed to open PTY master: " << strerror(errno);
        throw std::runtime_error("Failed to open PTY master");
    }
    Logger(Logger::Level::Info) << "PTY master opened successfully";

    if (grantpt(master_fd_raw_) == -1 || unlockpt(master_fd_raw_) == -1 || (slave_name = ptsname(master_fd_raw_)) == nullptr) {
            Logger(Logger::Level::Error) << "Failed to grant or unlock PTY: " << strerror(errno);
            throw std::runtime_error("Failed to grant or unlock PTY");
    }
    Logger(Logger::Level::Info) << "PTY grant and unlock successful";
    Logger(Logger::Level::Info) << "Slave PTY name: " << slave_name << std::endl;

    // Attempt to create a symbolic link from slave_name to "/dev/ttyUSB0"
    if (symlink(slave_name, device_name_.c_str()) == -1) {
        Logger(Logger::Level::Error) << "Failed to create symlink for PTY slave: " << strerror(errno);
        throw std::runtime_error("Failed to create symlink for PTY slave");
    }
    Logger(Logger::Level::Info) << "Symlink for PTY slave created successfully";

    // Open the slave pseudoterminal
    slave_fd_raw_ = open(slave_name, O_RDWR);
    if (slave_fd_raw_ == -1) {
        Logger(Logger::Level::Error) << "Failed to create symlink for PTY slave: " << strerror(errno);
        throw std::runtime_error("Failed to open the slave pseudoterminal");
    }

    chmod(device_name_.c_str(), 0660);
    struct group* tty_grp = getgrnam("tty");
    if (tty_grp && chown(device_name_.c_str(), -1, tty_grp->gr_gid) == -1) {
        Logger(Logger::Level::Error) << "Failed to change group of device: " << strerror(errno);
        throw std::runtime_error("Failed to change group of device");
    }
    Logger(Logger::Level::Info) << "Group changed successfully for the device";

    setup_pty(master_fd_raw_);
    setup_pty(slave_fd_raw_);
}

void VirtualSerialPort::setup_pty(int fd) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        Logger(Logger::Level::Error) << "Error from tcgetattr: " << strerror(errno);
        return;
    }

    cfmakeraw(&tty);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        Logger(Logger::Level::Error) << "Error from tcsetattr: " << strerror(errno);
    } else {
        Logger(Logger::Level::Info) << "PTY attributes set successfully";
    }
}

VirtualSerialPort::~VirtualSerialPort() {
    close(master_fd_raw_);
    close(slave_fd_raw_);
    unlink(device_name_.c_str());

}

ssize_t VirtualSerialPort::async_read(char* buffer, unsigned int length) {
    // Logger(Logger::Level::Info) << "VSP::async_read";
    return read(master_fd_raw_, buffer, length);
}


ssize_t VirtualSerialPort::async_write(const char* buffer, unsigned int length) {
    // Logger(Logger::Level::Info) << "VSP::async_write";
    return write(master_fd_raw_, buffer, length);
}

