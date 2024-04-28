#ifndef CLIENT_H
#define CLIENT_H

#include "common.hpp"
#include "VirtualSerialPort.h"

using namespace boost::asio;
using ip::tcp;
using std::string;

class SerialClient {
public:
    SerialClient(const string& server_ip, unsigned short server_port, const string& vsp_name);
    void run();

private:
    io_service io_service_;
    tcp::socket socket_;
    std::array<char, 256> buffer_;
    VirtualSerialPort vsp_;

    void do_read_write();
};


#endif // CLIENT_H
