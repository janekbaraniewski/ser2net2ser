#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>

#include "logging.h"

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
        BOOST_LOG_SEV(lg, logging::trivial::info) << "Initializing server with device: " << dev_name << ", baud rate: " << baud_rate << ", port: " << port;

        // Open the serial port
        try {
            serial_.open(dev_name);
            serial_.set_option(serial_port_base::baud_rate(baud_rate));
            BOOST_LOG_SEV(lg, logging::trivial::info) << "Serial port opened and configured.";
        } catch (boost::system::system_error& e) {
            BOOST_LOG_SEV(lg, logging::trivial::error) << "Failed to open serial port: " << e.what();
            throw;
        }

        // Start accepting connections
        start_accept();
    }

    void run() {
        BOOST_LOG_SEV(lg, logging::trivial::info) << "Server is running.";
        io_service_.run();
        BOOST_LOG_SEV(lg, logging::trivial::info) << "Server stopped.";
    }

private:
    void start_accept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec) {
                BOOST_LOG_SEV(lg, logging::trivial::info) << "Client connected.";
                do_read_write();
            } else {
                BOOST_LOG_SEV(lg, logging::trivial::error) << "Error accepting connection: " << ec.message();
            }
        });
    }

    void do_read_write() {
        static boost::array<char, 128> buf;

        // Asynchronous read from serial port
        serial_.async_read_some(boost::asio::buffer(buf), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                BOOST_LOG_SEV(lg, logging::trivial::info) << "Read " << length << " bytes from the serial port.";
                // Write to TCP socket
                async_write(socket_, boost::asio::buffer(buf, length), [this](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        BOOST_LOG_SEV(lg, logging::trivial::info) << "Data written to client.";
                        do_read_write();  // Loop back to continue reading/writing
                    } else {
                        BOOST_LOG_SEV(lg, logging::trivial::error) << "Error writing to client: " << ec.message();
                    }
                });
            } else {
                BOOST_LOG_SEV(lg, logging::trivial::error) << "Error reading from serial port: " << ec.message();
            }
        });

        // Asynchronous read from TCP socket
        socket_.async_read_some(boost::asio::buffer(buf), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                BOOST_LOG_SEV(lg, logging::trivial::info) << "Received " << length << " bytes from client.";
                // Write to serial port
                async_write(serial_, boost::asio::buffer(buf, length), [this](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        BOOST_LOG_SEV(lg, logging::trivial::info) << "Data written to serial port.";
                        do_read_write();  // Loop back to continue reading/writing
                    } else {
                        BOOST_LOG_SEV(lg, logging::trivial::error) << "Error writing to serial port: " << ec.message();
                    }
                });
            } else {
                BOOST_LOG_SEV(lg, logging::trivial::error) << "Error reading from client: " << ec.message();
            }
        });
    }
};

int main(int argc, char* argv[]) {
    init_logging();
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
        BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
    }
    return 0;
}