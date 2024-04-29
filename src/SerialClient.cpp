#include "SerialClient.h"

using namespace boost::asio;
using namespace boost::program_options;
using ip::tcp;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

SerialClient::SerialClient(boost::asio::io_service& io_service, const std::string& server_ip, unsigned short server_port, const std::string& vsp_name)
    : io_service_(io_service), socket_(io_service), vsp_(io_service, vsp_name) {
    BOOST_LOG_TRIVIAL(info) << "Initializing client...";
    boost::asio::ip::tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({server_ip, std::to_string(server_port)});
    BOOST_LOG_TRIVIAL(info) << "Connecting to server at " << server_ip << ":" << server_port;
    connect(socket_, endpoint_iterator);
    BOOST_LOG_TRIVIAL(info) << "Connected to server.";
    BOOST_LOG_TRIVIAL(info) << "Opening virtual serial port: " << vsp_name;
}

void SerialClient::run() {
    BOOST_LOG_TRIVIAL(info) << "Starting client I/O operations.";
    do_read_socket();
    do_read_vsp();
    io_service_.run();
}

void SerialClient::do_read_socket() {
    socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
        if (!ec && length > 0) {
            std::string data(buffer_.data(), length);
            std::fill(std::begin(buffer_), std::end(buffer_), 0);

            BOOST_LOG_TRIVIAL(debug) << "Received from server: " << data;

            vsp_.async_write(boost::asio::buffer(data), [this](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    do_read_socket();
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Write to VSP failed: " << ec.message();
                }
            });
        } else if (ec) {
            BOOST_LOG_TRIVIAL(error) << "Read error on socket: " << ec.message();
        }
    });
}

void SerialClient::do_read_vsp() {
    vsp_.async_read(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
        if (!ec && length > 0) {
            std::string data(buffer_.data(), length);
            std::fill(std::begin(buffer_), std::end(buffer_), 0);

            BOOST_LOG_TRIVIAL(debug) << "Received from VSP: " << data;

            async_write(socket_, boost::asio::buffer(data), [this](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    do_read_vsp();
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Write to socket failed: " << ec.message();
                }
            });
        } else if (ec) {
            BOOST_LOG_TRIVIAL(error) << "Read error on VSP: " << ec.message();
        }
    });
}
