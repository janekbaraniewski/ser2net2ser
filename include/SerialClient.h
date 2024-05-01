#ifndef CLIENT_H
#define CLIENT_H

#include "common.hpp"
#include "VirtualSerialPort.h"

using namespace boost::asio;
using ip::tcp;
using std::string;


class SocketClient {
    struct sockaddr_in server_addr;

public:
    int sock_fd;
    SocketClient() : sock_fd(-1) {
        memset(&server_addr, 0, sizeof(server_addr));
    }

    ~SocketClient() {
        close(sock_fd);
    }

    bool connectToServer(const char* server_ip, int server_port) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd == -1) {
            std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
            return false;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address/ Address not supported" << std::endl;
            return false;
        }

        if (::connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection Failed: " << strerror(errno) << std::endl;
            return false;
        }

        std::cout << "Connected to server at " << server_ip << ":" << server_port << std::endl;
        return true;
    }

    ssize_t sendToServer(const char* buffer, size_t bufferSize) {
        return send(sock_fd, buffer, bufferSize, 0);
    }

    ssize_t receiveFromServer(char* buffer, size_t bufferSize) {
        return recv(sock_fd, buffer, bufferSize, 0);
    }
};

class SerialClient {
private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 256> buffer_;
    VirtualSerialPort vsp_;

public:
    SerialClient(boost::asio::io_service& io_service, const std::string& server_ip, unsigned short server_port, const std::string& vsp_name);
    SocketClient socketClient_;
    void run();
    void do_read_vsp();
    void do_read_socket();
};

#endif // CLIENT_H
