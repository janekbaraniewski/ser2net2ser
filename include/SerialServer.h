#ifndef SERIALSERVER_H
#define SERIALSERVER_H

#include "RealSerialPort.h"
#include <boost/asio.hpp>
#include <array>

class SerialServer {
public:
    SerialServer(boost::asio::io_service& io_service, const std::string& device, unsigned int baud_rate);

    void run();

private:
    void start_accept();
    void handle_session();
    void async_read_socket();
    void async_read_serial();

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    RealSerialPort serial_port_;
    std::array<char, 1024> buffer_;
};

#endif // SERIALSERVER_H
