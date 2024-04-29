#include "SerialServer.h"
#include <boost/log/trivial.hpp>

SerialServer::SerialServer(boost::asio::io_service& io_service, const std::string& device, unsigned int baud_rate)
    : io_service_(io_service),
      acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 3333)),
      socket_(io_service),
      serial_port_(io_service) {
        serial_port_.open(device, boost::asio::serial_port_base::baud_rate(baud_rate));
    }

void SerialServer::run() {
    BOOST_LOG_TRIVIAL(info) << "SerialServer::run";
    start_accept();
    io_service_.run();
}

void SerialServer::start_accept() {
    BOOST_LOG_TRIVIAL(info) << "SerialServer::start_accept";

    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
        if (!ec) {
            handle_session();
        }
        start_accept();
    });
}

void SerialServer::handle_session() {
    BOOST_LOG_TRIVIAL(info) << "SerialServer::handle_session";

    async_read_socket();
    async_read_serial();
}

void SerialServer::async_read_socket() {
    BOOST_LOG_TRIVIAL(info) << "SerialServer::async_read_socket";

    socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
            serial_port_.async_write(boost::asio::buffer(buffer_, length), [this](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    async_read_socket();
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Error writing to client: " << ec.message();
                }
            });
        } else {
            BOOST_LOG_TRIVIAL(error) << "Read error on socket: " << ec.message();
            socket_.close();
        }
    });
}

void SerialServer::async_read_serial() {
    BOOST_LOG_TRIVIAL(info) << "SerialServer::async_read_serial";

    serial_port_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
            boost::asio::async_write(socket_, boost::asio::buffer(buffer_, length), [this](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    async_read_serial();
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Error sending to socket: " << ec.message();
                }
            });
        } else {
            BOOST_LOG_TRIVIAL(error) << "Read error on serial port: " << ec.message();
            socket_.close();
        }
    });
}
