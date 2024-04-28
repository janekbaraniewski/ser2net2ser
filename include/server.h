#ifndef SERVER_H
#define SERVER_H

#include "common.hpp"
#include "ISerialPort.h"
#include "RealSerialPort.h"

using namespace boost::asio;
using ip::tcp;
using std::string;

class SerialServer {
public:
    SerialServer(io_service& io, ISerialPort& serial, tcp::acceptor& acceptor);
    void run();

private:
    io_service& io_service_;
    ISerialPort& serial_;
    tcp::acceptor& acceptor_;
    tcp::socket socket_;

    void start_accept();
    void do_read_write();
};

#endif // SERVER_H
