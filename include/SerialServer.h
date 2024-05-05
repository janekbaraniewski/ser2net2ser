#ifndef SERIALSERVER_H
#define SERIALSERVER_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <array>

#include "RealSerialPort.h"
#include "SerialPort.h"
#include "TCPServer.h"

class SerialServer {
public:
    SerialServer(const std::string& device, unsigned int baudRate, unsigned int port);
    ~SerialServer();

    void run();

private:
    SerialPort serial_port_;
    TcpServer server_;
};

#endif // SERIALSERVER_H
