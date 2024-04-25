#include <iostream>
#include <boost/asio.hpp>
#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::cerr;
using std::endl;

class SerialClient {
private:
    io_service io_service_;
    tcp::socket socket_;
    int master_fd_, slave_fd_;
    std::array<char, 256> buffer_;

public:
    SerialClient(const std::string& server_ip, unsigned short server_port)
        : socket_(io_service_) {
        // Connect to server
        tcp::resolver resolver(io_service_);
        auto endpoint_iterator = resolver.resolve({server_ip, std::to_string(server_port)});
        connect(socket_, endpoint_iterator);

        // Setup PTY
        master_fd_ = posix_openpt(O_RDWR | O_NOCTTY);
        if (master_fd_ == -1 || grantpt(master_fd_) != 0 || unlockpt(master_fd_) != 0) {
            throw std::runtime_error("Failed to open or configure PTY master");
        }

        char* slave_name = ptsname(master_fd_);
        if (!slave_name) {
            throw std::runtime_error("Failed to get PTY slave name");
        }
        slave_fd_ = open(slave_name, O_RDWR);
        if (slave_fd_ == -1) {
            throw std::runtime_error("Failed to open PTY slave");
        }
        cout << "PTY setup completed. Slave device: " << slave_name << endl;
    }

    void run() {
        do_read_write();
        io_service_.run();
    }

private:
    void do_read_write() {
        // Asynchronous read from TCP socket
        socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                // Write to PTY master
                if (write(master_fd_, buffer_.data(), length) < 0) {
                    cerr << "Write to PTY master failed" << endl;
                    return;
                }
                // Optionally echo back to TCP socket for full duplex operation
                async_write(socket_, boost::asio::buffer(buffer_, length), [this](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        do_read_write();
                    } else {
                        cerr << "Write back to TCP socket failed: " << ec.message() << endl;
                    }
                });
            } else {
                cerr << "Read error: " << ec.message() << endl;
            }
        });
    }
};

int main() {
    try {
        SerialClient client("127.0.0.1", 12345);  // Server IP and port
        client.run();
    } catch (std::exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
