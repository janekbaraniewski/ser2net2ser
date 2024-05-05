#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "Logger.h"
#include "SerialPort.h"
#include <netinet/in.h>

class TcpServer {
private:
    int server_fd_;
    struct sockaddr_in address_;
    int port_;
    bool is_running_;
    SerialPort& serial_;

    static void handleClient(int client_socket, SerialPort& serial);

public:
    TcpServer(int port, SerialPort& serial);
    ~TcpServer();
    void run();
    void stop();
};

#endif // TCPSERVER_H
