#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include "SerialPort.h"
#include "Logger.h"

class TcpServer {
public:
    TcpServer(int port, SerialPort& serial);
    ~TcpServer();

    void run();
    void stop();

private:
    void handleClient(int client_socket);

    int server_fd_;
    int port_;
    bool is_running_;
    struct sockaddr_in address_;
    SerialPort& serial_;
};

#endif // TCPSERVER_H
