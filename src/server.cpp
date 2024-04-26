#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

using namespace boost::asio;
using namespace boost::program_options;
using ip::tcp;
using std::string;
using std::cout;
using std::cerr;
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
        static boost::array<char, 128> buf;

        serial_.async_read_some(buffer(buf), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                async_write(socket_, buffer(buf, length), [this](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        do_read_write();  // Loop back to continue reading/writing
                    }
                });
            }
        });

        socket_.async_read_some(buffer(buf), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                async_write(serial_, buffer(buf, length), [this](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        do_read_write();  // Loop back to continue reading/writing
                    }
                });
            }
        });
    }
};

int main(int argc, char* argv[]) {
    try {
        options_description desc{"Options"};
        desc.add_options()
            ("help,h", "Help screen")
            ("device,d", value<string>()->default_value("/dev/ttyUSB0"), "Device name")
            ("baud,b", value<unsigned int>()->default_value(9600), "Baud rate")
            ("port,p", value<unsigned short>()->default_value(12345), "Port number");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 0;
        }

        string dev_name = vm["device"].as<string>();
        unsigned int baud_rate = vm["baud"].as<unsigned int>();
        unsigned short port = vm["port"].as<unsigned short>();

        SerialServer server(dev_name, baud_rate, port);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
