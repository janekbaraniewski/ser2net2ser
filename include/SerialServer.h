#ifndef SERIALSERVER_H
#define SERIALSERVER_H

#include "RealSerialPort.h"
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class SerialServer {
public:
    SerialServer(const std::string& device, unsigned int baudRate, unsigned int port);
    ~SerialServer();

    void run();

private:
    int server_sock;
    int client_sock;
    RealSerialPort serial_port_;
    std::array<char, 1024> buffer_;

    void start_accept();
    void handle_session(int client_sock);
    void async_read_socket(int client_sock);
    void async_read_serial(int client_sock);
};

#endif // SERIALSERVER_H
