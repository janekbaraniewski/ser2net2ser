#include "SerialServer.h"

using namespace boost::asio;
using namespace boost::program_options;
using ip::tcp;
using std::string;
using std::endl;

SerialServer::SerialServer(io_service& io, ISerialPort& serial, tcp::acceptor& acceptor)
    : io_service_(io), serial_(serial), acceptor_(acceptor), socket_(io) {
    BOOST_LOG_TRIVIAL(info) << "Starting server and waiting for connection...";
    start_accept();
}

void SerialServer::run() {
    BOOST_LOG_TRIVIAL(info) << "Server is running.";
    io_service_.run();
    BOOST_LOG_TRIVIAL(info) << "Server stopped.";
}

void SerialServer::start_accept() {
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
        if (!ec) {
            BOOST_LOG_TRIVIAL(info) << "Client connected. Starting to handle read/write operations.";
            do_read_write();
        } else {
            BOOST_LOG_TRIVIAL(error) << "Error accepting connection: " << ec.message();
        }
    });
}

void SerialServer::do_read_write() {
    static boost::array<char, 128> buf;
    serial_.async_read_some(boost::asio::buffer(buf), [this](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
            async_write(socket_, boost::asio::buffer(buf, length), [this](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    BOOST_LOG_TRIVIAL(info) << "Data successfully written to client. Continuing read/write loop.";
                    do_read_write();
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Error writing to client: " << ec.message();
                }
            });
        } else {
            BOOST_LOG_TRIVIAL(error) << "Error reading from serial port: " << ec.message();
        }
    });
}

#ifndef UNIT_TEST
int main(int argc, char* argv[]) {
    init_logging();
    io_service io;
    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12345));
    RealSerialPort realSerial(io);

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
            std::cout << desc << std::endl;
            return 0;
        }

        string dev_name = vm["device"].as<string>();
        unsigned int baud_rate = vm["baud"].as<unsigned int>();
        unsigned short port = vm["port"].as<unsigned short>();

        realSerial.open(dev_name);
        realSerial.set_option(serial_port_base::baud_rate(baud_rate));

        SerialServer server(io, realSerial, acceptor);
        server.run();
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
        return 1; // Ensure returning a non-zero value on error
    }
    return 0;
}
#else
#error "UNIT_TEST is defined!"
#endif
