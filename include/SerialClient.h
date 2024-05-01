#ifndef CLIENT_H
#define CLIENT_H

#include "common.hpp"
#include "VirtualSerialPort.h"
#include "SocketClient.h"

using namespace boost::asio;
using ip::tcp;
using std::string;

class SerialClient {
private:
    std::array<char, 256> buffer_;
    VirtualSerialPort vsp_;

public:
    SerialClient(boost::asio::io_service& io_service, const std::string& server_ip, unsigned short server_port, const std::string& vsp_name);
    SocketClient socketClient_;
    void run();
};

#endif // CLIENT_H
