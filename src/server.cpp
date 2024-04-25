#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::endl;

class SerialServer {
private:
    io_service io_service_;
    serial_port serial_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;

public:
    SerialServer(const string& dev_name, unsigned int baud_rate, unsigned short port)
        : serial_(io_service_), socket_(io_service_), acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)) {
        // Open the serial port
        serial_.open(dev_name);
        serial_.set_option(serial_port_base::baud_rate(baud_rate));

        // Start accepting connections
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                cout << "Client connected." << endl;
                do_read_write();
            }
        });
    }

    void run() {
        io_service_.run();
    }

private:
    void do_read_write() {
        // Communication buffers
        static boost::array<char, 128> buf;

        // Asynchronous read from serial port
        serial_.async_read_some(buffer(buf), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                // Write to TCP socket
                async_write(socket_, buffer(buf, length), [this](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        do_read_write();  // Loop back to continue reading/writing
                    }
                });
            }
        });

        // Asynchronous read from TCP socket
        socket_.async_read_some(buffer(buf), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                // Write to serial port
                async_write(serial_, buffer(buf, length), [this](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        do_read_write();  // Loop back to continue reading/writing
                    }
                });
            }
        });
    }
};

int main() {
    try {
        SerialServer server("/dev/ttyUSB0", 9600, 12345);  // Modify as needed
        server.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
