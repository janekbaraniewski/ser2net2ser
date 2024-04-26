#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

using namespace boost::asio;
using namespace boost::program_options;
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
        socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                if (write(master_fd_, buffer_.data(), length) < 0) {
                    cerr << "Write to PTY master failed" << endl;
                    return;
                }
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

int main(int argc, char* argv[]) {
    try {
        options_description desc{"Options"};
        desc.add_options()
            ("help,h", "Help screen")
            ("server,s", value<std::string>()->default_value("127.0.0.1"), "Server IP address")
            ("port,p", value<unsigned short>()->default_value(12345), "Server port");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 0;
        }

        std::string server_ip = vm["server"].as<std::string>();
        unsigned short server_port = vm["port"].as<unsigned short>();

        SerialClient client(server_ip, server_port);
        client.run();
    } catch (const std::exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
