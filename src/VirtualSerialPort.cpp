#include "VirtualSerialPort.h"

VirtualSerialPort::VirtualSerialPort(boost::asio::io_context& io_context, const std::string& device)
    : slave_fd_(io_context), device_name_("/dev/" + device) {
    std::lock_guard<std::mutex> lock(mutex_);
    int slave_fd;
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
    slave_fd = open(slave_name, O_RDWR);
    if (slave_fd == -1) {
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

    slave_fd_.assign(slave_fd);
    setup_pty(master_fd_raw_);
    setup_pty(slave_fd);
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
    slave_fd_.close();
    unlink(device_name_.c_str());

}

ssize_t VirtualSerialPort::async_read(char* buffer, unsigned int length) {
    Logger(Logger::Level::Info) << "VSP::async_read";
    ssize_t bytes_read = read(master_fd_raw_, buffer, length);
    Logger(Logger::Level::Info) << "READ FROM SERIAL!!!! -> ";
    return bytes_read;
}


ssize_t VirtualSerialPort::async_write(const char* buffer, unsigned int length) {
    return write(master_fd_raw_, buffer, length);
}

