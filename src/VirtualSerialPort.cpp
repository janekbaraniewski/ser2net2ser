#include "VirtualSerialPort.h"

VirtualSerialPort::VirtualSerialPort(boost::asio::io_context& io_context, const std::string& device)
    : master_fd_(io_context), slave_fd_(io_context), device_name_("/dev/" + device) {
    std::lock_guard<std::mutex> lock(mutex_);
    int master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1) {
        BOOST_LOG_TRIVIAL(error) << "Failed to open PTY master: " << strerror(errno);
        throw std::runtime_error("Failed to open PTY master");
    }
        BOOST_LOG_TRIVIAL(info) << "PTY master opened successfully";

        if (grantpt(master_fd) != 0 || unlockpt(master_fd) != 0) {
            BOOST_LOG_TRIVIAL(error) << "Failed to grant or unlock PTY: " << strerror(errno);
            throw std::runtime_error("Failed to grant or unlock PTY");
        }
        BOOST_LOG_TRIVIAL(info) << "PTY grant and unlock successful";

        char* slave_name = ptsname(master_fd);
        if (!slave_name) {
            BOOST_LOG_TRIVIAL(error) << "Failed to get PTY slave name";
            throw std::runtime_error("Failed to get PTY slave name");
        }

        if (unlink(device_name_.c_str()) == -1 && errno != ENOENT) {
            BOOST_LOG_TRIVIAL(error) << "Failed to remove existing symlink: " << strerror(errno);
            throw std::runtime_error("Failed to remove existing symlink");
        }
        BOOST_LOG_TRIVIAL(info) << "Existing symlink removed or not present";

        BOOST_LOG_TRIVIAL(info) << "Slave PTY name: " << slave_name << std::endl;

        // Open the slave pseudoterminal
        int slave_fd;
        slave_fd = open(slave_name, O_RDWR);
        if (slave_fd == -1) {
            std::cerr << "Error opening slave PTY: " << strerror(errno) << std::endl;
            close();
            return;
        }
        slave_fd_.assign(slave_fd);

        if (symlink(slave_name, device_name_.c_str()) != 0) {
            BOOST_LOG_TRIVIAL(error) << "Failed to create symlink for PTY slave: " << strerror(errno);
            throw std::runtime_error("Failed to create symlink for PTY slave");
        }
        BOOST_LOG_TRIVIAL(info) << "Symlink for PTY slave created successfully";

        chmod(device_name_.c_str(), 0660);
        struct group* tty_grp = getgrnam("tty");
        if (tty_grp && chown(device_name_.c_str(), -1, tty_grp->gr_gid) == -1) {
            BOOST_LOG_TRIVIAL(error) << "Failed to change group of device: " << strerror(errno);
            throw std::runtime_error("Failed to change group of device");
        }
        BOOST_LOG_TRIVIAL(info) << "Group changed successfully for the device";

        master_fd_.assign(master_fd);
        setup_pty(master_fd);
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

void VirtualSerialPort::async_read(boost::asio::mutable_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    BOOST_LOG_TRIVIAL(info) << "VSP::async_read";
    // boost::asio::async_read(master_fd_, buffer,
    //     [this, buffer, handler](const boost::system::error_code& ec, std::size_t length) {
    //         if (!ec && length > 0) {
    //             BOOST_LOG_TRIVIAL(info) << "VSP::async_read::success";
    //             std::string data(boost::asio::buffer_cast<const char*>(buffer), length);
    //             std::stringstream hex_stream;
    //             hex_stream << std::hex << std::setfill('0');
    //             for(unsigned char c : data) {
    //                 hex_stream << std::setw(2) << static_cast<int>(c) << " ";
    //             }
    //             BOOST_LOG_TRIVIAL(info) << "Read from VSP: " << hex_stream.str();

    //             handler(ec, length);
    //         } else if (ec) {
    //             BOOST_LOG_TRIVIAL(error) << "Read error on VSP: " << ec.message();
    //             handler(ec, 0);
    //         }
    //     });
    char buffer_[256];
    ssize_t bytes_read;
    while ((bytes_read = read(master_fd_.native_handle(), buffer_, sizeof(buffer_) - 1)) > 0) {
        buffer_[bytes_read] = '\0'; // Null-terminate string
        BOOST_LOG_TRIVIAL(info) << "READ FROM SERIAL!!!! -> " << buffer_;
    }

    if (bytes_read == -1) {
        std::cerr << "Error reading from PTY: " << strerror(errno) << std::endl;
    }
}


void VirtualSerialPort::async_write(boost::asio::const_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    std::lock_guard<std::mutex> lock(mutex_);
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
