#ifndef SERVER_H
#define SERVER_H

#include "common.hpp"
#include "ISerialPort.h"
#include "RealSerialPort.h"

using namespace boost::asio;
using ip::tcp;
using std::string;

class SerialServer {
private:
    boost::asio::io_service& io_service_;
    ISerialPort& serial_;
    boost::asio::ip::tcp::acceptor& acceptor_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 1024> buf;  // Buffer for data

    void start_accept();
    void do_read_write();
public:
    SerialServer(boost::asio::io_service& io, ISerialPort& serial, boost::asio::ip::tcp::acceptor& acceptor);
    void run();
};

#endif // SERVER_H
