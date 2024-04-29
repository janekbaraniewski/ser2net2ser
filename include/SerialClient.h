#ifndef CLIENT_H
#define CLIENT_H

#include "common.hpp"
#include "VirtualSerialPort.h"

using namespace boost::asio;
using ip::tcp;
using std::string;

class SerialClient {
private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 256> buffer_;
    VirtualSerialPort vsp_;  // Declare the VirtualSerialPort object

public:
    SerialClient(boost::asio::io_service& io_service, const std::string& server_ip, unsigned short server_port, const std::string& vsp_name);
    void run();
    void do_read_write();
};

#endif // CLIENT_H
