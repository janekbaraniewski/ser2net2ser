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
    chown(device_name_.c_str(), -1, getgrnam("tty")->gr_gid);

    master_fd_.assign(master_fd);
}



VirtualSerialPort::~VirtualSerialPort() {
    close();
}

void VirtualSerialPort::close() {
    master_fd_.close();
    unlink(device_name_.c_str());
}

void VirtualSerialPort::async_read(boost::asio::mutable_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    boost::asio::async_read(master_fd_, buffer, handler);
}

void VirtualSerialPort::async_write(boost::asio::const_buffer buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
    boost::asio::async_write(master_fd_, buffer, handler);
}

// bool VirtualSerialPort::write(const std::string& data) {
//     if (master_fd_ == -1) {
//         BOOST_LOG_TRIVIAL(error) << "Attempt to write with closed master FD";
//         return false;
//     }

//     ssize_t written = 0;
//     ssize_t to_write = data.size();
//     while (written < to_write) {
//         ssize_t result = ::write(master_fd_, data.c_str() + written, to_write - written);
//         if (result < 0) {
//             BOOST_LOG_TRIVIAL(error) << "Write error: " << strerror(errno);
//             return false;
//         }
//         written += result;
//     }
//     return true;
// }

// std::string VirtualSerialPort::read() {
//     if (master_fd_ == -1) {
//         BOOST_LOG_TRIVIAL(error) << "Attempt to read with closed master FD";
//         return "";
//     }

//     char buffer[256];
//     ssize_t len = ::read(master_fd_, buffer, sizeof(buffer) - 1);
//     if (len < 0) {
//         BOOST_LOG_TRIVIAL(error) << "Read error: " << strerror(errno);
//         return "";
//     } else if (len == 0) {
//         BOOST_LOG_TRIVIAL(info) << "No data read";
//         return "";
//     }

//     buffer[len] = '\0';
//     return std::string(buffer, len);
// }
