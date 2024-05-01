#include "VirtualSerialPort.h"

VirtualSerialPort::VirtualSerialPort(boost::asio::io_context& io_context, const std::string& device)
    : master_fd_(io_context), slave_fd_(io_context), device_name_("/dev/" + device) {
    std::lock_guard<std::mutex> lock(mutex_);
    int master_fd, slave_fd;
    char* slave_name;
    master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1) {
        BOOST_LOG_TRIVIAL(error) << "Failed to open PTY master: " << strerror(errno);
        throw std::runtime_error("Failed to open PTY master");
    }
    BOOST_LOG_TRIVIAL(info) << "PTY master opened successfully";

    if (grantpt(master_fd) == -1 || unlockpt(master_fd) == -1 || (slave_name = ptsname(master_fd)) == nullptr) {
            BOOST_LOG_TRIVIAL(error) << "Failed to grant or unlock PTY: " << strerror(errno);
            throw std::runtime_error("Failed to grant or unlock PTY");
    }
    BOOST_LOG_TRIVIAL(info) << "PTY grant and unlock successful";
    BOOST_LOG_TRIVIAL(info) << "Slave PTY name: " << slave_name << std::endl;

    // Attempt to create a symbolic link from slave_name to "/dev/ttyUSB0"
    if (symlink(slave_name, device_name_.c_str()) == -1) {
        BOOST_LOG_TRIVIAL(error) << "Failed to create symlink for PTY slave: " << strerror(errno);
        throw std::runtime_error("Failed to create symlink for PTY slave");
    }
    BOOST_LOG_TRIVIAL(info) << "Symlink for PTY slave created successfully";

    // Open the slave pseudoterminal
    slave_fd = open(slave_name, O_RDWR);
    if (slave_fd == -1) {
        BOOST_LOG_TRIVIAL(error) << "Failed to create symlink for PTY slave: " << strerror(errno);
        throw std::runtime_error("Failed to open the slave pseudoterminal");
    }



    chmod(device_name_.c_str(), 0660);
    struct group* tty_grp = getgrnam("tty");
    if (tty_grp && chown(device_name_.c_str(), -1, tty_grp->gr_gid) == -1) {
        BOOST_LOG_TRIVIAL(error) << "Failed to change group of device: " << strerror(errno);
        throw std::runtime_error("Failed to change group of device");
    }
    BOOST_LOG_TRIVIAL(info) << "Group changed successfully for the device";

    master_fd_.assign(master_fd);
    slave_fd_.assign(slave_fd);
    setup_pty(master_fd);
    setup_pty(slave_fd);
}

void VirtualSerialPort::setup_pty(int fd) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        BOOST_LOG_TRIVIAL(error) << "Error from tcgetattr: " << strerror(errno);
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
        BOOST_LOG_TRIVIAL(error) << "Error from tcsetattr: " << strerror(errno);
    } else {
        BOOST_LOG_TRIVIAL(info) << "PTY attributes set successfully";
    }
}

void VirtualSerialPort::close() {
    master_fd_.close();
    slave_fd_.close();
    unlink(device_name_.c_str());
}

VirtualSerialPort::~VirtualSerialPort() {
    close();
}

ssize_t VirtualSerialPort::async_read(char* buffer, unsigned int length) {
    BOOST_LOG_TRIVIAL(info) << "VSP::async_read";
    ssize_t bytes_read = read(master_fd_.native_handle(), buffer, length);
    BOOST_LOG_TRIVIAL(info) << "READ FROM SERIAL!!!! -> ";
    return bytes_read;
}


ssize_t VirtualSerialPort::async_write(const char* buffer, unsigned int length) {
    return write(master_fd_.native_handle(), buffer, length);
}

