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
            start_accept();
        }
    });
}

void SerialServer::do_read_write() {
    // Asynchronous read from serial port
    serial_.async_read_some(boost::asio::buffer(buf), [this](boost::system::error_code ec, std::size_t length) {
        if (!ec && length > 0) {
            // Data is available
            async_write(socket_, boost::asio::buffer(buf, length), [this](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    BOOST_LOG_TRIVIAL(info) << "Data successfully written to client.";
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Error writing to client: " << ec.message();
                }
            });
        } else if (!ec) {
            BOOST_LOG_TRIVIAL(info) << "No data received from serial. Waiting...";
        } else {
            BOOST_LOG_TRIVIAL(error) << "Error reading from serial port: " << ec.message();
        }
        do_read_write();  // Continue the loop
    });
}
