#include "VirtualSerialPort.h"

VirtualSerialPort::VirtualSerialPort(boost::asio::io_context& io_context, const std::string& device)
    : master_fd_(io_context), device_name_("/dev/" + device) {
    int master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1 || grantpt(master_fd) != 0 || unlockpt(master_fd) != 0) {
        throw std::runtime_error("Failed to open PTY master");
    }

    char* slave_name = ptsname(master_fd);
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
    struct group* tty_grp = getgrnam("tty");
    if (tty_grp) {
        chown(device_name_.c_str(), -1, tty_grp->gr_gid);
    }

    master_fd_.assign(master_fd);
    setup_pty(master_fd);
}


VirtualSerialPort::~VirtualSerialPort() {
    close();
}

void VirtualSerialPort::setup_pty(int fd) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        BOOST_LOG_TRIVIAL(error) << "Error from tcgetattr: " << strerror(errno);
        return;
    }

    cfmakeraw(&tty); // Configure the terminal attributes to raw mode

    tty.c_cflag |= (CLOCAL | CREAD); // Ignore modem controls and enable receiver
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; // 8-bit characters
    tty.c_cflag &= ~PARENB; // No parity bit
    tty.c_cflag &= ~CSTOPB; // Only need 1 stop bit
    tty.c_cflag &= ~CRTSCTS; // No hardware flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Disable canonical mode, echo, and signal chars
    tty.c_oflag &= ~OPOST; // No output processing

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        BOOST_LOG_TRIVIAL(error) << "Error from tcsetattr: " << strerror(errno);
    }
}

void VirtualSerialPort::close() {
    master_fd_.close();
    unlink(device_name_.c_str());
}

void VirtualSerialPort::async_read(boost::asio::mutable_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    boost::asio::async_read(master_fd_, buffer,
        [this, buffer, handler](const boost::system::error_code& ec, std::size_t length) {
            if (!ec && length > 0) {
                std::string data(boost::asio::buffer_cast<const char*>(buffer), length);
                std::stringstream hex_stream;
                hex_stream << std::hex << std::setfill('0');
                for(unsigned char c : data) {
                    hex_stream << std::setw(2) << static_cast<int>(c) << " ";
                }
                BOOST_LOG_TRIVIAL(info) << "Read from VSP: " << hex_stream.str();

                handler(ec, length);
            } else if (ec) {
                BOOST_LOG_TRIVIAL(error) << "Read error on VSP: " << ec.message();
                handler(ec, 0);
            }
        });
}


void VirtualSerialPort::async_write(boost::asio::const_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    std::string data(boost::asio::buffer_cast<const char*>(buffer), boost::asio::buffer_size(buffer));
    std::stringstream hex_stream;
    hex_stream << std::hex << std::setfill('0');
    for (unsigned char c : data) {
        hex_stream << std::setw(2) << static_cast<int>(c) << " ";
    }
    // BOOST_LOG_TRIVIAL(info) << "Writing to VSP: " << hex_stream.str();

    boost::asio::async_write(master_fd_, buffer,
        [this, handler](const boost::system::error_code& ec, std::size_t length) {
            if (!ec) {
                // BOOST_LOG_TRIVIAL(info) << "Successfully written " << length << " bytes to VSP";
            } else {
                BOOST_LOG_TRIVIAL(error) << "Write error on VSP: " << ec.message();
            }
            handler(ec, length);
        });
}
